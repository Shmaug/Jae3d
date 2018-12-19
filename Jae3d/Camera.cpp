#include "Camera.hpp"

#include <DirectXMath.h>
#include "Graphics.hpp"
#include "ConstantBuffer.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "CommandQueue.hpp"
#include "CommandList.hpp"
#include "AssetDatabase.hpp"
#include "Profiler.hpp"
#include "Light.hpp"
#include "Scene.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

struct GPULight {
	XMFLOAT3 position;
	float range;
	XMFLOAT3 color;
	float intensity;
};

Camera::Camera(jwstring name) : Object(name), mFieldOfView(70.0f), mNear(.1f), mFar(1000.0f) {
	mPixelWidth = Graphics::GetWindow()->GetWidth();
	mPixelHeight = Graphics::GetWindow()->GetHeight();

	mCBuffer = shared_ptr<ConstantBuffer>(new ConstantBuffer(sizeof(XMFLOAT4X4) * 3 + sizeof(XMFLOAT4) + sizeof(uint32_t), L"Camera CB"));
	mLightBuffer = shared_ptr<ConstantBuffer>(new ConstantBuffer(sizeof(GPULight) * 64, L"Camera Light CB"));

	mLightIndexTexture = new shared_ptr<Texture>[Graphics::BufferCount()];
}
Camera::~Camera() { delete[] mLightIndexTexture; }

void Camera::WriteCBuffer(unsigned int frameIndex){
	UpdateTransform();

	unsigned int o = 0;
	mCBuffer->WriteFloat4x4(mView, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat4x4(mProjection, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat4x4(mViewProjection, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat4x4(mInverseProj, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat3(WorldPosition(), o, frameIndex); o += sizeof(XMFLOAT4);
	mCBuffer->WriteFloat4(XMFLOAT4((float)mPixelWidth, (float)mPixelHeight, mNear, mFar), o, frameIndex);
}

void Camera::CalculateScreenLights(Scene* scene, unsigned int frameIndex) {
	Profiler::BeginSample(L"Calculate tiled lighting");
	unsigned int w = mPixelWidth / 4;
	unsigned int h = mPixelHeight / 4;
	if (!mLightIndexTexture[frameIndex] || mLightIndexTexture[frameIndex]->Width() != w || mLightIndexTexture[frameIndex]->Height() != h) {
		uint32_t* data = new uint32_t[w * h * 2];
		ZeroMemory(data, w * h * sizeof(uint32_t) * 2);

		mLightIndexTexture[frameIndex] = shared_ptr<Texture>(new Texture(
			L"Light Index Texture", w, h, 1,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, DXGI_FORMAT_R32G32_UINT, ALPHA_MODE_OTHER, 1,
			data, w * h * sizeof(uint32_t) * 2, false));
		mLightIndexTexture[frameIndex]->Upload(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		delete[] data;
	}

	static jvector<Light*> lights;
	lights.clear();
	scene->CollectLights(mFrustum, lights);

	unsigned int count = min(64, (int)lights.size());
	GPULight glights[64];
	ZeroMemory(&glights, sizeof(GPULight) * 64);
	for (unsigned int i = 0; i < count; i++) {
		glights[i].position = lights[i]->WorldPosition();
		glights[i].range = lights[i]->mRange;
		XMStoreFloat3(&glights[i].color, XMLoadFloat3(&lights[i]->mColor) * lights[i]->mIntensity);
		glights[i].intensity = lights[i]->mIntensity;
	}

	XMFLOAT4 groundCol = { .025f, .025f, .025f, 0 };
	XMFLOAT4 skyCol = { .05f, .05f, .05f, 0 };

	unsigned int o = 0;
	mLightBuffer->Write(&glights, sizeof(GPULight) * 64, o, frameIndex); o += sizeof(GPULight) * 64;
	mLightBuffer->WriteUInt(count, o, frameIndex); o += sizeof(uint32_t);
	mLightBuffer->WriteUInt(w, o, frameIndex); o += sizeof(uint32_t);
	mLightBuffer->WriteUInt(h, o, frameIndex); o += sizeof(uint32_t);
	o += sizeof(uint32_t);
	mLightBuffer->WriteFloat4(groundCol, o, frameIndex); o += sizeof(XMFLOAT4);
	mLightBuffer->WriteFloat4(skyCol, o, frameIndex);

	shared_ptr<Shader> lightCompute = AssetDatabase::GetAsset<Shader>(L"LightCompute");
	auto cmdQueue = Graphics::GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	auto cmdList = cmdQueue->GetCommandList(frameIndex);
	
	cmdList->SetCompute(lightCompute);
	cmdList->D3DCommandList()->SetComputeRootConstantBufferView(1, mCBuffer->GetGPUAddress(frameIndex));
	cmdList->D3DCommandList()->SetComputeRootConstantBufferView(2, mLightBuffer->GetGPUAddress(frameIndex));
	
	ID3D12DescriptorHeap* heap = { mLightIndexTexture[frameIndex]->GetUAVDescriptorHeap().Get() };
	cmdList->D3DCommandList()->SetDescriptorHeaps(1, &heap);
	cmdList->D3DCommandList()->SetComputeRootDescriptorTable(0, mLightIndexTexture[frameIndex]->GetUAVGPUDescriptor());

	cmdList->D3DCommandList()->Dispatch((w + 7) / 8, (h + 7) / 8, 1);
	
	cmdQueue->WaitForFenceValue(cmdQueue->Execute(cmdList));
	Profiler::EndSample();
}

bool Camera::UpdateTransform(){
	if (!Object::UpdateTransform()) return false;

	XMVECTOR worldPos = XMLoadFloat3(&WorldPosition());
	XMVECTOR worldRot = XMLoadFloat4(&WorldRotation());

	XMMATRIX view = XMMatrixLookToLH(worldPos, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), worldRot), XMVector3Rotate(XMVectorSet(0, 1, 0, 0), worldRot));
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(mFieldOfView), (float)mPixelWidth / (float)mPixelHeight , mNear, mFar);

	XMStoreFloat4x4(&mView, view);
	XMStoreFloat4x4(&mProjection, proj);
	XMStoreFloat4x4(&mViewProjection, view * proj);
	XMStoreFloat4x4(&mInverseProj, XMMatrixInverse(&XMMatrixDeterminant(proj), proj));

	BoundingFrustum::CreateFromMatrix(mFrustum, proj);
	mFrustum.Orientation = WorldRotation();
	mFrustum.Origin = WorldPosition();

	return true;
}