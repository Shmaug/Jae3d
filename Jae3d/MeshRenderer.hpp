#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <memory>

#include "Object.hpp"
#include "Renderer.hpp"

class CommandList;
class Mesh;
class Material;
class ConstantBuffer;

class MeshRenderer : public Renderer {
public:
	JAE_API MeshRenderer();
	JAE_API MeshRenderer(jwstring name);
	JAE_API ~MeshRenderer();

	JAE_API void Draw(std::shared_ptr<CommandList> commandList);

	std::shared_ptr<Material> mMaterial;
	std::shared_ptr<Mesh> mMesh;

private:
	std::shared_ptr<ConstantBuffer> mCBuffer;
};

