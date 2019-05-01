#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <memory>

#include "Object.hpp"
#include "Renderer.hpp"

class Mesh;
class ConstantBuffer;

class MeshRenderer : public Renderer {
public:
	class SubmeshRenderJob : public RenderJob {
	public:
		DirectX::XMFLOAT4X4 mObjectToWorld;
		unsigned int mSubmesh;
		std::shared_ptr<Mesh> mMesh;
		std::shared_ptr<Material> mMaterial;
		std::shared_ptr<ConstantBuffer> mObjectBuffer;

		JAE_API SubmeshRenderJob(unsigned int queue, const jwstring& batch, const std::shared_ptr<Mesh>& mesh, unsigned int submesh, const std::shared_ptr<Material>& mat, const std::shared_ptr<ConstantBuffer>& buf, const DirectX::XMFLOAT4X4& o2w);

		JAE_API RenderJob* Batch(RenderJob* other, const std::shared_ptr<CommandList>& commandList) override;
		JAE_API void Execute(const std::shared_ptr<CommandList>& commandList, const std::shared_ptr<Material>& materialOverride) override;
	};

	JAE_API MeshRenderer();
	JAE_API MeshRenderer(const jwstring& name);
	JAE_API ~MeshRenderer();

	JAE_API void SetMesh(const std::shared_ptr<Mesh>& mesh);
	inline std::shared_ptr<Mesh> GetMesh() { return mMesh; };
	inline void SetMaterial(const std::shared_ptr<Material>& material, unsigned int submesh = 0) { mMaterials[submesh] = material; };
	inline std::shared_ptr<Material> GetMaterial(unsigned int submesh = 0) { return mMaterials[submesh]; };

	virtual JAE_API void GatherRenderJobs(const std::shared_ptr<CommandList>& commandList, const std::shared_ptr<Camera>& camera, jvector<RenderJob*> &list) override;
	virtual bool Visible() override { return mVisible; }
	virtual JAE_API DirectX::BoundingOrientedBox Bounds() override;

	bool mVisible;
	jwstring mBatchGroup;

private:
	std::shared_ptr<ConstantBuffer> mCBuffer;

	jvector<std::shared_ptr<Material>> mMaterials;
	std::shared_ptr<Mesh> mMesh;
};

