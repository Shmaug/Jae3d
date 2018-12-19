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

DirectX::BoundingOrientedBox MeshRenderer::Bounds() {
	if (mMesh) {
		XMFLOAT3 mcenter = mMesh->Bounds().Center;
		XMFLOAT3 mbounds = mMesh->Bounds().Extents;
		XMFLOAT3 scale = WorldScale();
		XMVECTOR p = XMVector3Transform(XMLoadFloat3(&mcenter), XMLoadFloat4x4(&ObjectToWorld()));
		XMStoreFloat3(&mcenter, p);
		return BoundingOrientedBox(mcenter, XMFLOAT3(mbounds.x * scale.x, mbounds.y * scale.y, mbounds.z * scale.z), WorldRotation());
	}
	return BoundingOrientedBox(WorldPosition(), XMFLOAT3(0, 0, 0), WorldRotation());
}

void MeshRenderer::Draw(shared_ptr<CommandList> commandList) {
	if (!mMesh || !mMaterial) return;
	UpdateTransform();

	mCBuffer->WriteFloat4x4(ObjectToWorld(), 0, commandList->GetFrameIndex());
	mCBuffer->WriteFloat4x4(WorldToObject(), sizeof(XMFLOAT4X4), commandList->GetFrameIndex());

	mMaterial->SetCBuffer(L"ObjectBuffer", mCBuffer, commandList->GetFrameIndex());
	commandList->SetMaterial(mMaterial);
	commandList->DrawMesh(*mMesh);
}