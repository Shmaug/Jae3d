#include "MeshRenderer.hpp"
#include "Graphics.hpp"

#include "CommandList.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "ConstantBuffer.hpp"
#include "Profiler.hpp"
#include "Shader.hpp"

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

MeshRenderer::SubmeshRenderJob::SubmeshRenderJob(unsigned int queue, const jwstring& batch, const shared_ptr<Mesh>& mesh, unsigned int submesh, const shared_ptr<Material>& mat, const shared_ptr<ConstantBuffer>& buf, const XMFLOAT4X4& o2w)
	: RenderJob(queue), mMesh(mesh), mSubmesh(submesh), mMaterial(mat), mObjectBuffer(buf), mObjectToWorld(o2w) {
	mName = L"MeshRenderer (" + mMaterial->mName + L", " + mMaterial->GetShader()->mName + L')';
	if (!batch.empty())
		mBatchGroup = L"MeshRenderer " + batch;
}

Renderer::RenderJob* MeshRenderer::SubmeshRenderJob::Batch(RenderJob* other, const shared_ptr<CommandList>& commandList) {
	// compare material parameters
	SubmeshRenderJob* j = dynamic_cast<SubmeshRenderJob*>(other);
	if (!j) return nullptr;

	// TODO
	//shared_ptr<Mesh> mesh = shared_ptr<Mesh>(new Mesh(L"Batch Mesh"));
	//shared_ptr<ConstantBuffer> cbuf = shared_ptr<ConstantBuffer>(new ConstantBuffer(sizeof(XMFLOAT4) * 2, L"Batch CB", 3));
	
	//XMFLOAT4X4 identity(
	//	1,0,0,0,
	//	0,1,0,0,
	//	0,0,1,0,
	//	0,0,0,1 );
	
	//cbuf->WriteFloat4x4(identity, 0, commandList->GetFrameIndex());
	//cbuf->WriteFloat4x4(identity, sizeof(XMFLOAT4X4), commandList->GetFrameIndex());
	
	//return new SubmeshRenderJob(mRenderQueue, mBatchGroup, mesh, 0, mMaterial, cbuf, identity);
	return nullptr;
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
		list.push_back(new SubmeshRenderJob(mMaterials[i]->RenderQueue(), mBatchGroup, mMesh, i, mMaterials[i], mCBuffer, ObjectToWorld()));
	}
}