#include "MeshRenderer.hpp"
#include "Graphics.hpp"

#include "CommandList.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "ConstantBuffer.hpp"
#include "Profiler.hpp"

using namespace Microsoft::WRL;
using namespace std;
using namespace DirectX;

MeshRenderer::MeshRenderer() : MeshRenderer(L"") {}
MeshRenderer::MeshRenderer(const jwstring& name) : Renderer(name), mVisible(true) {
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
void MeshRenderer::SetMesh(const shared_ptr<Mesh>& mesh) {
	mMesh = mesh;
	if (mMesh) {
		while (mMaterials.size() < mesh->SubmeshCount())
			mMaterials.push_back(nullptr);
	}
}

bool MeshRenderer::SubmeshRenderJob::LessThan(RenderJob* b) {
	if (RenderJob::LessThan(b)) return true;
	return false;
	SubmeshRenderJob* j = dynamic_cast<SubmeshRenderJob*>(b);
	if (!j) return true;
	return j->mMaterial->mName < mMaterial->mName;
}
void MeshRenderer::SubmeshRenderJob::Execute(const shared_ptr<CommandList>& commandList, const std::shared_ptr<Material>& materialOverride){
	if (materialOverride) mMaterial = materialOverride;
	mMaterial->SetCBuffer("ObjectBuffer", mObjectBuffer, commandList->GetFrameIndex());
	commandList->SetMaterial(mMaterial);
	commandList->DrawMesh(*mMesh, mSubmesh);
}

void MeshRenderer::GatherRenderJobs(const shared_ptr<CommandList>& commandList, const shared_ptr<Camera>& camera, jvector<RenderJob*>& list) {
	if (!mMesh) return;
	UpdateTransform();

	mCBuffer->WriteFloat4x4(ObjectToWorld(), 0, commandList->GetFrameIndex());
	mCBuffer->WriteFloat4x4(WorldToObject(), sizeof(XMFLOAT4X4), commandList->GetFrameIndex());

	for (unsigned int i = 0; i < mMesh->SubmeshCount(); i++) {
		if (!mMaterials[i]) continue;
		list.push_back(new SubmeshRenderJob(mMaterials[i]->RenderQueue(), mMesh, i, mMaterials[i], mCBuffer));
	}
}