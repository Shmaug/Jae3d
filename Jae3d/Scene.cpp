#include "Scene.hpp"
#include "CommandList.hpp"
#include "Light.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"

#include "Mesh.hpp"
#include "Shader.hpp"
#include "AssetDatabase.hpp"
#include "Profiler.hpp"

#include <algorithm>

using namespace std;
using namespace DirectX;

Scene::Scene() : mDebugShader(nullptr), mDebugCube(nullptr), mSkyCube(nullptr) {}
Scene::~Scene() {
	mObjects.free();
	mRenderers.free();
	mLights.free();
}

void Scene::DebugDrawBox(const shared_ptr<CommandList>& commandList, const BoundingOrientedBox &box, const XMMATRIX &vp) {
	if (!mDebugCube) {
		mDebugCube = std::shared_ptr<Mesh>(new Mesh(L"Cube"));
		mDebugCube->Topology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		mDebugCube->VertexCount(8);
		auto verts = mDebugCube->GetVertices();
		unsigned int i = 0;
		verts[i++] = XMFLOAT3(-1.0f,  1.0f, -1.0f);
		verts[i++] = XMFLOAT3(-1.0f,  1.0f,  1.0f);
		verts[i++] = XMFLOAT3( 1.0f,  1.0f,  1.0f);
		verts[i++] = XMFLOAT3( 1.0f,  1.0f, -1.0f);

		verts[i++] = XMFLOAT3(-1.0f, -1.0f, -1.0f);
		verts[i++] = XMFLOAT3(-1.0f, -1.0f,  1.0f);
		verts[i++] = XMFLOAT3( 1.0f, -1.0f,  1.0f);
		verts[i++] = XMFLOAT3( 1.0f, -1.0f, -1.0f);
		unsigned int indices[24]{
			0, 1, 1, 2, 2, 3, 3, 0,
			4, 5, 5, 6, 6, 7, 7, 4,
			0, 4, 1, 5, 2, 6, 3, 7
		};
		mDebugCube->AddIndices(24, indices);
		mDebugCube->UploadStatic();
	}

	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, XMMatrixAffineTransformation(XMLoadFloat3(&box.Extents), XMVectorZero(), XMLoadFloat4(&box.Orientation), XMLoadFloat3(&box.Center)) * vp);
	commandList->D3DCommandList()->SetGraphicsRoot32BitConstants(0, 16, &m, 0);
	commandList->DrawMesh(*mDebugCube);
}

void Scene::DrawSkybox(const shared_ptr<CommandList>& commandList) {
	if (!mSkybox) return;
	if (!mSkyCube) {
		mSkyCube = shared_ptr<Mesh>(new Mesh(L"Sky Cube"));
		mSkyCube->LoadCube(1.0f);
		mSkyCube->UploadStatic();
	}
	commandList->PushState();
	commandList->SetMaterial(mSkybox);
	commandList->DrawMesh(*mSkyCube);
	commandList->PopState();
}
void Scene::Draw(const shared_ptr<CommandList>& commandList, const shared_ptr<Camera>& camera, const shared_ptr<Material>& materialOverride) {
	static jvector<Renderer::RenderJob*> renderQueue;

	const BoundingFrustum &cullFrustum = camera->Frustum();

	Profiler::BeginSample(L"Gather Renderer Jobs");
	renderQueue.clear();
	for (int i = 0; i < mRenderers.size(); i++) {
		if (!mRenderers[i]->Visible() || !mRenderers[i]->Bounds().Intersects(cullFrustum)) continue;
		mRenderers[i]->GatherRenderJobs(commandList, camera, renderQueue);
	}
	Profiler::EndSample();

	Profiler::BeginSample(L"Sort Renderers");
	std::sort(renderQueue.data(), renderQueue.data() + renderQueue.size(), [](Renderer::RenderJob*& a, Renderer::RenderJob*& b) {
		return a->LessThan(b);
	});
	Profiler::EndSample();

	Profiler::BeginSample(L"Execute Renderer Jobs");
	for (int i = 0; i < renderQueue.size(); i++) {
		renderQueue[i]->Execute(commandList, materialOverride);
		delete renderQueue[i];
	}
	Profiler::EndSample();
}
void Scene::DebugDraw(const shared_ptr<CommandList>& commandList, const shared_ptr<Camera>& camera) {
	if (!mDebugShader) mDebugShader = AssetDatabase::GetAsset<Shader>(L"ColoredRoot");

	const BoundingFrustum &cullFrustum = camera->Frustum();

	commandList->PushState();

	commandList->SetShader(mDebugShader);
	commandList->SetBlendState(BLEND_STATE_ALPHA);
	commandList->SetFillMode(D3D12_FILL_MODE_WIREFRAME);

	XMMATRIX vp = XMLoadFloat4x4(&camera->ViewProjection());

	static XMFLOAT4 white(1.f, 1.f, 1.f, .5f);
	commandList->D3DCommandList()->SetGraphicsRoot32BitConstants(0, 4, &white, 16);

	for (unsigned int i = 0; i < mRenderers.size(); i++){
		if (!mRenderers[i]->Visible() || !mRenderers[i]->Bounds().Intersects(cullFrustum)) continue;
		DebugDrawBox(commandList, mRenderers[i]->Bounds(), vp);
	}

	for (unsigned int i = 0; i < mLights.size(); i++) {
		if (mLights[i]->mMode == Light::LIGHTMODE_DIRECTIONAL || !mLights[i]->Bounds().Intersects(cullFrustum)) continue;
		commandList->D3DCommandList()->SetGraphicsRoot32BitConstants(0, 3, &mLights[i]->mColor, 16);
		DebugDrawBox(commandList, mLights[i]->Bounds(), vp);
	}

	commandList->PopState();
}

void Scene::CollectLights(const BoundingFrustum &frustum, jvector<Light*> &lights) {
	for (int i = 0; i < mLights.size(); i++)
		if (mLights[i]->mMode == Light::LIGHTMODE_DIRECTIONAL || mLights[i]->Bounds().Intersects(frustum))
			lights.push_back(mLights[i]);
}

void Scene::Intersect(const DirectX::XMVECTOR& point, jvector<std::shared_ptr<Object>>& result) const {
	for (int i = 0; i < mObjects.size(); i++)
		if (mObjects[i]->Bounds().Contains(point))
			result.push_back(mObjects[i]);
}
void Scene::Intersect(const DirectX::XMFLOAT3& point, jvector<std::shared_ptr<Object>>& result) const {
	XMVECTOR pt = XMLoadFloat3(&point);
	for (int i = 0; i < mObjects.size(); i++)
		if (mObjects[i]->Bounds().Contains(pt))
			result.push_back(mObjects[i]);
}
void Scene::Intersect(const DirectX::BoundingOrientedBox& bounds, jvector<std::shared_ptr<Object>>& result) const {
	for (int i = 0; i < mObjects.size(); i++)
		if (mObjects[i]->Bounds().Intersects(bounds))
			result.push_back(mObjects[i]);
}
void Scene::Intersect(const DirectX::BoundingSphere& sphere, jvector<std::shared_ptr<Object>>& result) const {
	for (int i = 0; i < mObjects.size(); i++)
		if (mObjects[i]->Bounds().Intersects(sphere))
			result.push_back(mObjects[i]);
}