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
		unsigned int mSubmesh;
		std::shared_ptr<Mesh> mMesh;
		std::shared_ptr<Material> mMaterial;
		std::shared_ptr<ConstantBuffer> mObjectBuffer;

		SubmeshRenderJob(unsigned int queue, std::shared_ptr<Mesh> mesh, unsigned int submesh, std::shared_ptr<Material> mat, std::shared_ptr<ConstantBuffer> buf)
			: RenderJob(queue), mMesh(mesh), mSubmesh(submesh), mMaterial(mat), mObjectBuffer(buf) {}

		JAE_API void Execute(std::shared_ptr<CommandList> commandList, std::shared_ptr<Material> materialOverride) override;
		JAE_API bool LessThan(RenderJob* b) override;
	};

	JAE_API MeshRenderer();
	JAE_API MeshRenderer(jwstring name);
	JAE_API ~MeshRenderer();

	JAE_API void SetMesh(std::shared_ptr<Mesh> mesh);
	inline std::shared_ptr<Mesh> GetMesh() { return mMesh; };
	inline void SetMaterial(std::shared_ptr<Material> material, unsigned int submesh = 0) { mMaterials[submesh] = material; };
	inline std::shared_ptr<Material> GetMaterial(unsigned int submesh = 0) { return mMaterials[submesh]; };

	JAE_API void GatherRenderJobs(std::shared_ptr<CommandList> commandList, std::shared_ptr<Camera> camera, jvector<RenderJob*> &list) override;
	bool Visible() override { return mVisible; }
	DirectX::BoundingOrientedBox Bounds() override;

	bool mVisible;

private:
	std::shared_ptr<ConstantBuffer> mCBuffer;

	jvector<std::shared_ptr<Material>> mMaterials;
	std::shared_ptr<Mesh> mMesh;
};

