#pragma once

#include <memory>
#include "Object.hpp"
#include <DirectXCollision.h>

class CommandList;
class Material;
class Camera;

class Renderer : public Object {
public:
	// Inherit from this to create a render job
	// Render jobs are sorted according to LessThan() and executed
	class RenderJob {
	public:
		unsigned int mRenderQueue;
		jwstring mName;
		jwstring mBatchGroup;

		RenderJob(unsigned int queue) : mRenderQueue(queue), mName(L"RenderJob"), mBatchGroup(L"") {}

		virtual RenderJob* Batch(RenderJob* other, const std::shared_ptr<CommandList>& commandList) { return nullptr; };
		virtual void Execute(const std::shared_ptr<CommandList>& commandList, const std::shared_ptr<Material>& materialOverride) {};
		virtual bool LessThan(const RenderJob* b) const {
			return mRenderQueue < b->mRenderQueue || (mRenderQueue == b->mRenderQueue && mBatchGroup < b->mBatchGroup);
		}
	};

	Renderer() : Object(L"") {}
	Renderer(const jwstring& name) : Object(name) {}
	~Renderer() {}

	// Collect the RenderJobs this Renderer will execute
	// For example: MeshRenderer creates a RenderJob for each Submesh (for different materials)
	virtual void GatherRenderJobs(const std::shared_ptr<CommandList>& commandList, const std::shared_ptr<Camera>& camera, jvector<RenderJob*>& list) = 0;
	virtual bool Visible() = 0;

};