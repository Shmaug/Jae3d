#include "Lighting.hpp"

#include <Graphics.hpp>
#include <Profiler.hpp>
#include <AssetDatabase.hpp>
#include <CommandList.hpp>

using namespace std;
using namespace DirectX;

Lighting::Lighting() {
	mLightBuffer = shared_ptr<ConstantBuffer>(new ConstantBuffer(sizeof(LightingBuffer), L"Camera Light CB", Graphics::BufferCount()));
	mLightIndexTexture = new shared_ptr<Texture>[Graphics::BufferCount()];
	mShadowAtlas = new shared_ptr<Texture>[Graphics::BufferCount()];
	mLightingTextureTable = new shared_ptr<DescriptorTable>[Graphics::BufferCount()];

	ZeroMemory(mLightIndexTexture, sizeof(shared_ptr<Texture>) * Graphics::BufferCount());
	ZeroMemory(mShadowAtlas, sizeof(shared_ptr<Texture>) * Graphics::BufferCount());
	ZeroMemory(mLightingTextureTable, sizeof(shared_ptr<Texture>) * Graphics::BufferCount());

	mDepthMaterial = shared_ptr<Material>(new Material(L"DepthPass", AssetDatabase::GetAsset<Shader>(L"DepthPass")));
	mDepthMaterial->CullMode(D3D12_CULL_MODE_FRONT);
	mLightCompute = AssetDatabase::GetAsset<Shader>(L"LightCompute");
}
Lighting::~Lighting() { delete[] mLightIndexTexture; }

shared_ptr<Camera> Lighting::RenderShadows(shared_ptr<CommandList> commandList, shared_ptr<Camera> cam, Light* light, shared_ptr<Scene> scene, XMUINT4 vp) {
	unsigned int frameIndex = commandList->GetFrameIndex();
	if (!mShadowCamera) {
		mShadowCameraMount = scene->AddObject<Object>(L"Shadow Camera Mount");
		mShadowCamera = scene->AddObject<Camera>(L"Shadow Camera");
		mShadowCamera->Parent(mShadowCameraMount);
		mShadowCamera->RenderFormat(DXGI_FORMAT_R32_FLOAT);
		mShadowCamera->PixelWidth(4096);
		mShadowCamera->PixelHeight(4096);
		mShadowCamera->SampleCount(1);
		mShadowCamera->CreateRenderBuffers();
	}
	mShadowCameraMount->LocalRotation(light->WorldRotation());
	if (light->mMode == Light::LIGHTMODE_DIRECTIONAL) {
		mShadowCamera->LocalPosition(0, 0, -32.0f);
		mShadowCameraMount->LocalPosition(cam->WorldPosition());
		mShadowCamera->Orthographic(true);
		mShadowCamera->OrthographicSize(16.0f);
		mShadowCamera->Near(1.0f);
		mShadowCamera->Far(1024.0f);
	} else {
		mShadowCamera->LocalPosition(0, 0, 0);
		mShadowCameraMount->LocalPosition(light->WorldPosition());
		mShadowCamera->Orthographic(false);
		mShadowCamera->Near(.01f);
		mShadowCamera->Far(light->mRange);
		mShadowCamera->FieldOfView(light->mSpotAngle);
	}

	commandList->SetCamera(mShadowCamera, CD3DX12_VIEWPORT((float)vp.x, (float)vp.y, (float)vp.z, (float)vp.w));
	mShadowCamera->Clear(commandList);
	// TODO: scene culling not working correctly with shadow camera?
	scene->Draw(commandList, mShadowCamera, mDepthMaterial);

	commandList->TransitionResource(mShadowAtlas[frameIndex]->GetTexture().Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->TransitionResource(mShadowCamera->RenderBuffer().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	commandList->D3DCommandList()->CopyResource(mShadowAtlas[frameIndex]->GetTexture().Get(), mShadowCamera->RenderBuffer().Get());
	commandList->TransitionResource(mShadowCamera->RenderBuffer().Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->TransitionResource(mShadowAtlas[frameIndex]->GetTexture().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	return mShadowCamera;
}

void Lighting::CalculateScreenLights(shared_ptr<CommandList> commandList, shared_ptr<Camera> camera, shared_ptr<Scene> scene) {
	unsigned int frameIndex = commandList->GetFrameIndex();
	if (!mShadowAtlas[frameIndex]) {
		mShadowAtlas[frameIndex] = shared_ptr<Texture>(new Texture(L"Shadow Atlas", 4096, 4096, DXGI_FORMAT_R32_FLOAT));
		mShadowAtlas[frameIndex]->Upload();
	}
	if (!mLightingTextureTable[frameIndex]) {
		mLightingTextureTable[frameIndex] = shared_ptr<DescriptorTable>(new DescriptorTable(2));
		mLightingTextureTable[frameIndex]->SetSRV(1, mShadowAtlas[frameIndex]);
	}
	unsigned int w = camera->PixelWidth() / 4;
	unsigned int h = camera->PixelHeight() / 4;
	if (!mLightIndexTexture[frameIndex] || mLightIndexTexture[frameIndex]->Width() != w || mLightIndexTexture[frameIndex]->Height() != h) {
		uint32_t* data = new uint32_t[w * h * 2];
		ZeroMemory(data, w * h * sizeof(uint32_t) * 2);

		mLightIndexTexture[frameIndex] = shared_ptr<Texture>(new Texture(
			L"Light Index Texture", w, h, 1,
			D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, DXGI_FORMAT_R32G32_UINT, 1,
			data, w * h * sizeof(uint32_t) * 2, false));
		mLightIndexTexture[frameIndex]->Upload(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, true);

		mLightingTextureTable[frameIndex]->SetSRV(0, mLightIndexTexture[frameIndex]);

		delete[] data;
	}

	static jvector<Light*> lights;
	lights.clear();
	scene->CollectLights(camera->Frustum(), lights);

	// sort so directional and spot lights always at the front (wont get omitted)
	sort(lights.data(), lights.data() + lights.size(), [](Light*& a, Light*& b) {
		return (a->mMode < b->mMode) || (a->mShadows && !b->mShadows);
	});

	mLightData.lightCount = min(64u, (unsigned int)lights.size());
	unsigned int s = 0;
	unsigned int sx = 0;
	unsigned int sy = 0;
	unsigned int sw = mShadowAtlas[frameIndex]->Width();
	unsigned int sh = mShadowAtlas[frameIndex]->Height();
	// TODO: update sx, sy, sw, sh to fit all shadow lights into the atlas
	for (unsigned int i = 0; i < mLightData.lightCount; i++) {
		mLightData.lights[i].position = lights[i]->WorldPosition();
		mLightData.lights[i].range = lights[i]->mRange;

		XMStoreFloat3(&mLightData.lights[i].color, XMLoadFloat3(&lights[i]->mColor) * lights[i]->mIntensity);
		mLightData.lights[i].mode = (float)lights[i]->mMode;

		XMStoreFloat3(&mLightData.lights[i].direction, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&lights[i]->WorldRotation())));
		mLightData.lights[i].angle = cosf(lights[i]->mSpotAngle);

		if (lights[i]->mShadows) {
			auto cam = RenderShadows(commandList, camera, lights[i], scene, { sx, sy, sw, sh });
			mLightData.shadows[s].view = cam->View();
			mLightData.shadows[s].viewProjection = cam->ViewProjection();
			mLightData.shadows[s].uv = { 0, 0, mShadowAtlas[frameIndex]->Width(), mShadowAtlas[frameIndex]->Height() };
			mLightData.shadows[s].bias = .03f;
			mLightData.shadows[s].normalBias = .02f;
			mLightData.shadows[s].strength = lights[i]->mShadowStrength;
			mLightData.lights[i].shadowIndex = s++;
		} else {
			mLightData.lights[i].shadowIndex = -1;
		}
	}

	mLightData.indexBufferSize = { w, h };
	mLightData.groundColor = { .025f, .025f, .025f, 0 };
	mLightData.skyColor = { .05f, .05f, .05f, 0 };

	mLightBuffer->Write(&mLightData, sizeof(LightingBuffer), 0, frameIndex);

	commandList->PushState();
	commandList->ClearKeywords();
	commandList->SetCompute(mLightCompute);
	commandList->D3DCommandList()->SetComputeRootConstantBufferView(1, camera->CBuffer()->GetGPUAddress(frameIndex));
	commandList->D3DCommandList()->SetComputeRootConstantBufferView(2, mLightBuffer->GetGPUAddress(frameIndex));

	ID3D12DescriptorHeap* heap = { mLightIndexTexture[frameIndex]->GetUAVDescriptorHeap().Get() };
	commandList->D3DCommandList()->SetDescriptorHeaps(1, &heap);
	commandList->D3DCommandList()->SetComputeRootDescriptorTable(0, mLightIndexTexture[frameIndex]->GetUAVGPUDescriptor());

	commandList->D3DCommandList()->Dispatch((w + 7) / 8, (h + 7) / 8, 1);

	commandList->SetGlobalCBuffer("LightBuffer", mLightBuffer);
	commandList->SetGlobalTable("LightingTextures", mLightingTextureTable[frameIndex]);
	commandList->PopState();
}

std::shared_ptr<Texture> Lighting::CalculateEnvironmentTexture() {
	// TODO: render cubemap
	return mEnvironmentTexture;
}