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

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

struct GPULight {
	XMFLOAT4 position;
	XMFLOAT4 color;
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

	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;
	XMFLOAT4X4 viewproj;
	XMFLOAT4X4 invp;
	XMFLOAT3 pos;
	XMFLOAT4 res{ (float)mPixelWidth, (float)mPixelHeight, mNear, mFar };

	XMStoreFloat4x4(&view, mView);
	XMStoreFloat4x4(&proj, mProjection);
	XMStoreFloat4x4(&viewproj, mViewProjection);
	XMStoreFloat4x4(&invp, mInverseProj);
	XMStoreFloat3(&pos, WorldPosition());

	unsigned int o = 0;
	mCBuffer->WriteFloat4x4(view, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat4x4(proj, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat4x4(viewproj, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat4x4(invp, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat3(pos, o, frameIndex); o += sizeof(XMFLOAT4);
	mCBuffer->WriteFloat4(res, o, frameIndex);
}

void Camera::CalculateScreenLights(jvector<std::shared_ptr<Light>> &lights, unsigned int frameIndex) {
	Profiler::BeginSample(L"Calculate tiled lighting");
	if (!mLightIndexTexture[frameIndex] || mLightIndexTexture[frameIndex]->Width() != mPixelWidth || mLightIndexTexture[frameIndex]->Height() != mPixelHeight) {
		uint32_t* data = new uint32_t[mPixelWidth * mPixelHeight * 2];
		ZeroMemory(data, mPixelWidth * mPixelHeight * sizeof(uint32_t) * 2);

		mLightIndexTexture[frameIndex] = shared_ptr<Texture>(new Texture(
			L"Light Index Texture", mPixelWidth, mPixelHeight, 1,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, DXGI_FORMAT_R32G32_UINT, ALPHA_MODE_OTHER, 1,
			data, mPixelWidth * mPixelHeight * sizeof(uint32_t) * 2, false));
		mLightIndexTexture[frameIndex]->Upload(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		delete[] data;
	}

	unsigned int count = min(64, (int)lights.size());
	GPULight glights[64];
	ZeroMemory(&glights, sizeof(GPULight) * 64);
	for (unsigned int i = 0; i < count; i++) {
		XMStoreFloat4(&glights[i].position, lights[i]->LocalPosition());
		glights[i].position.w = lights[i]->mRange;
		XMStoreFloat4(&glights[i].color, lights[i]->mColor * lights[i]->mIntensity);
		glights[i].color.w = lights[i]->mIntensity;
	}

	XMFLOAT4 groundCol = { .025f, .025f, .025f, 0 };
	XMFLOAT4 skyCol = { .05f, .05f, .05f, 0 };

	mLightBuffer->Write(&glights, sizeof(GPULight) * 64, 0, frameIndex);
	mLightBuffer->WriteUInt(count, sizeof(GPULight) * 64, frameIndex);
	mLightBuffer->WriteFloat4(groundCol, sizeof(GPULight) * 64 + sizeof(XMFLOAT4), frameIndex);
	mLightBuffer->WriteFloat4(skyCol, sizeof(GPULight) * 64 + sizeof(XMFLOAT4) * 2, frameIndex);

	shared_ptr<Shader> lightCompute = AssetDatabase::GetAsset<Shader>(L"LightCompute");
	auto cmdQueue = Graphics::GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	auto cmdList = cmdQueue->GetCommandList(frameIndex);
	
	cmdList->SetCompute(lightCompute);
	cmdList->D3DCommandList()->SetComputeRootConstantBufferView(1, mCBuffer->GetGPUAddress(frameIndex));
	cmdList->D3DCommandList()->SetComputeRootConstantBufferView(2, mLightBuffer->GetGPUAddress(frameIndex));
	
	ID3D12DescriptorHeap* heap = { mLightIndexTexture[frameIndex]->GetUAVDescriptorHeap().Get() };
	cmdList->D3DCommandList()->SetDescriptorHeaps(1, &heap);
	cmdList->D3DCommandList()->SetComputeRootDescriptorTable(0, mLightIndexTexture[frameIndex]->GetUAVGPUDescriptor());

	cmdList->D3DCommandList()->Dispatch((mPixelWidth + 7) / 8, (mPixelHeight + 7) / 8, 1);
	
	cmdQueue->WaitForFenceValue(cmdQueue->Execute(cmdList));
	Profiler::EndSample();
}

bool Camera::UpdateTransform(){
	if (!Object::UpdateTransform()) return false;

	XMVECTOR fwd = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), WorldRotation());
	XMVECTOR up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), WorldRotation());
	mView = XMMatrixLookToLH(WorldPosition(), fwd, up);
	mProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(mFieldOfView), (float)mPixelWidth / (float)mPixelHeight , mNear, mFar);
	mViewProjection = mView * mProjection;

	mInverseProj = XMMatrixInverse(&XMMatrixDeterminant(mProjection), mProjection);

	return true;
}