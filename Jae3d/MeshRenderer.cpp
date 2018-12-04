#include "MeshRenderer.hpp"
#include "Graphics.hpp"

#include "CommandList.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "ConstantBuffer.hpp"

using namespace Microsoft::WRL;
using namespace std;

MeshRenderer::MeshRenderer() : MeshRenderer("") {}
MeshRenderer::MeshRenderer(jstring name) : Object(name) { 
	mConstantBuffer = new ConstantBuffer(sizeof(ObjectBuffer), "MeshRenderer CB");
}
MeshRenderer::~MeshRenderer() { delete mConstantBuffer; }

bool MeshRenderer::UpdateTransform() {
	if (!Object::UpdateTransform()) return false;

	XMStoreFloat4x4(&mObjectBufferData.ObjectToWorld, ObjectToWorld());
	XMStoreFloat4x4(&mObjectBufferData.WorldToObject, WorldToObject());
	mConstantBuffer->Write(&mObjectBufferData, sizeof(ObjectBuffer));

	return true;
}
void MeshRenderer::Draw(shared_ptr<CommandList> commandList) {
	if (!mMesh || !mMaterial) return;
	UpdateTransform();
	commandList->SetMaterial(mMaterial);
	commandList->D3DCommandList()->SetGraphicsRootConstantBufferView(0, mConstantBuffer->GetGPUAddress());
	commandList->DrawMesh(*mMesh);
}