#include "MeshRenderer.hpp"
#include "Graphics.hpp"

#include "CommandList.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "ConstantBuffer.hpp"

using namespace Microsoft::WRL;
using namespace std;
using namespace DirectX;

MeshRenderer::MeshRenderer() : MeshRenderer(L"") {}
MeshRenderer::MeshRenderer(jwstring name) : Renderer(name) { 
	mCBuffer = shared_ptr<ConstantBuffer>(new ConstantBuffer(sizeof(XMFLOAT4X4) * 2, L"MeshRenderer CB", Graphics::BufferCount()));
}
MeshRenderer::~MeshRenderer() {}

void MeshRenderer::Draw(shared_ptr<CommandList> commandList) {
	if (!mMesh || !mMaterial) return;
	UpdateTransform();

	XMFLOAT4X4 o2w;
	XMFLOAT4X4 w2o;

	XMStoreFloat4x4(&o2w, ObjectToWorld());
	XMStoreFloat4x4(&w2o, WorldToObject());
	mCBuffer->WriteFloat4x4(o2w, 0, commandList->GetFrameIndex());
	mCBuffer->WriteFloat4x4(w2o, sizeof(XMFLOAT4X4), commandList->GetFrameIndex());

	mMaterial->SetCBuffer(L"ObjectBuffer", mCBuffer, commandList->GetFrameIndex());
	commandList->SetMaterial(mMaterial);
	commandList->DrawMesh(*mMesh);
}