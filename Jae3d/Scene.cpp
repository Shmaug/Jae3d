#include "Scene.hpp"
#include "CommandList.hpp"
#include "Light.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"

#include "Mesh.hpp"
#include "Shader.hpp"
#include "AssetDatabase.hpp"

using namespace DirectX;

Scene::Scene() {

}
Scene::~Scene() {
	mObjects.free();
	mRenderers.free();
	mLights.free();
}

void Scene::Draw(std::shared_ptr<CommandList> commandList, BoundingFrustum cullFrustum) {
	for (int i = 0; i < mRenderers.size(); i++) {
		if (!mRenderers[i]->mVisible || !mRenderers[i]->Bounds().Intersects(cullFrustum)) continue;
		mRenderers[i]->Draw(commandList);
	}
}
void Scene::DebugDraw(std::shared_ptr<CommandList> commandList, BoundingFrustum cullFrustum) {
	if (!mDebugCube) {
		mDebugCube = std::shared_ptr<Mesh>(new Mesh(L"Cube"));
		mDebugCube->LoadCube(1.0);
		
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

		mDebugShader = AssetDatabase::GetAsset<Shader>(L"Colored");
	}

	commandList->PushState();

	commandList->SetShader(mDebugShader);
	commandList->SetBlendState(BLEND_STATE_ALPHA);
	commandList->SetFillMode(D3D12_FILL_MODE_WIREFRAME);

	XMMATRIX vp = XMLoadFloat4x4(&commandList->GetCamera()->ViewProjection());

	static XMFLOAT4 white(1.f, 1.f, 1.f, .5f);
	commandList->D3DCommandList()->SetGraphicsRoot32BitConstants(0, 4, &white, 16);

	for (unsigned int i = 0; i < mRenderers.size(); i++){
		if (!mRenderers[i]->Bounds().Intersects(cullFrustum)) continue;
		DirectX::BoundingOrientedBox box = mRenderers[i]->Bounds();
		XMFLOAT4X4 m;
		XMStoreFloat4x4(&m, XMMatrixAffineTransformation(XMLoadFloat3(&box.Extents), XMVectorZero(), XMLoadFloat4(&box.Orientation), XMLoadFloat3(&box.Center)) * vp);
		commandList->D3DCommandList()->SetGraphicsRoot32BitConstants(0, 16, &m, 0);
		commandList->DrawUserMesh(MESH_SEMANTIC_POSITION, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		mDebugCube->Draw(commandList->D3DCommandList(), D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	commandList->PopState();
}

void Scene::CollectLights(BoundingFrustum &frustum, jvector<Light*> &lights) {
	for (int i = 0; i < mLights.size(); i++)
		if (mLights[i]->Bounds().Intersects(frustum))
			lights.push_back(mLights[i]);
}