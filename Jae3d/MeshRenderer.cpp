#include "MeshRenderer.hpp"
#include "Graphics.hpp"

#include "CommandList.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "ConstantBuffer.hpp"

using namespace Microsoft::WRL;
using namespace std;

MeshRenderer::MeshRenderer() : MeshRenderer(L"") {}
MeshRenderer::MeshRenderer(jwstring name) : Object(name) { 
	mConstantBuffer = new ConstantBuffer(sizeof(ObjectBuffer), L"MeshRenderer CB", Graphics::BufferCount());
}
MeshRenderer::~MeshRenderer() { delete mConstantBuffer; }

void MeshRenderer::Draw(shared_ptr<CommandList> commandList, unsigned int frameIndex) {
	if (!mMesh || !mMaterial) return;
	UpdateTransform();

	XMStoreFloat4x4(&mObjectBufferData.ObjectToWorld, ObjectToWorld());
	XMStoreFloat4x4(&mObjectBufferData.WorldToObject, WorldToObject());
	mConstantBuffer->Write(&mObjectBufferData, sizeof(ObjectBuffer), 0, frameIndex);

	commandList->SetMaterial(mMaterial);
	commandList->D3DCommandList()->SetGraphicsRootConstantBufferView(0, mConstantBuffer->GetGPUAddress(frameIndex));
	commandList->DrawMesh(*mMesh);
}