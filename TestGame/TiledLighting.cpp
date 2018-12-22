#include "TiledLighting.hpp"

#include <Graphics.hpp>
#include <Profiler.hpp>
#include <AssetDatabase.hpp>
#include <CommandQueue.hpp>
#include <CommandList.hpp>
#include <Shader.hpp>

using namespace std;
using namespace DirectX;

struct GPULight {
	XMFLOAT3 position;
	float range;
	XMFLOAT3 color;
	float intensity;
};

TiledLighting::TiledLighting() {
	mLightBuffer = shared_ptr<ConstantBuffer>(new ConstantBuffer(sizeof(GPULight) * 64, L"Camera Light CB", Graphics::BufferCount()));
	mLightIndexTexture = new shared_ptr<Texture>[Graphics::BufferCount()];
}
TiledLighting::~TiledLighting() { delete[] mLightIndexTexture; }

uint64_t TiledLighting::CalculateScreenLights(shared_ptr<Camera> camera, shared_ptr<Scene> scene, unsigned int frameIndex) {
	unsigned int w = camera->PixelWidth() / 4;
	unsigned int h = camera->PixelHeight() / 4;
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
	scene->CollectLights(camera->Frustum(), lights);

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
	cmdList->D3DCommandList()->SetComputeRootConstantBufferView(1, camera->CBuffer()->GetGPUAddress(frameIndex));
	cmdList->D3DCommandList()->SetComputeRootConstantBufferView(2, mLightBuffer->GetGPUAddress(frameIndex));

	ID3D12DescriptorHeap* heap = { mLightIndexTexture[frameIndex]->GetUAVDescriptorHeap().Get() };
	cmdList->D3DCommandList()->SetDescriptorHeaps(1, &heap);
	cmdList->D3DCommandList()->SetComputeRootDescriptorTable(0, mLightIndexTexture[frameIndex]->GetUAVGPUDescriptor());

	cmdList->D3DCommandList()->Dispatch((w + 7) / 8, (h + 7) / 8, 1);

	return cmdQueue->Execute(cmdList);
}