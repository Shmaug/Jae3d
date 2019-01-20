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
		unsigned int RenderQueue;

		RenderJob(unsigned int queue) : RenderQueue(queue) {}

		virtual void Execute(std::shared_ptr<CommandList> commandList, std::shared_ptr<Material> materialOverride) {};

		virtual bool LessThan(RenderJob* b) {
			return RenderQueue < b->RenderQueue;
		}
	};

	Renderer() : Object(L"") {}
	Renderer(jwstring name) : Object(name) {}
	~Renderer() {}

	// Collect the RenderJobs this Renderer will execute
	// For example: MeshRenderer creates a RenderJob for each Submesh (for different materials)
	virtual void GatherRenderJobs(std::shared_ptr<CommandList> commandList, std::shared_ptr<Camera> camera, jvector<RenderJob*> &list) = 0;
	virtual bool Visible() = 0;

};