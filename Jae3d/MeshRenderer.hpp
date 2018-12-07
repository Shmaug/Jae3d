#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <memory>

#include "Object.hpp"

class CommandList;
class Mesh;
class Material;
class ConstantBuffer;

class MeshRenderer : public Object {
public:
	JAE_API MeshRenderer();
	JAE_API MeshRenderer(jwstring name);
	JAE_API ~MeshRenderer();

	JAE_API void Draw(std::shared_ptr<CommandList> commandList, unsigned int frameIndex);

	std::shared_ptr<Material> mMaterial;
	std::shared_ptr<Mesh> mMesh;

private:
	struct ObjectBuffer {
	public:
		DirectX::XMFLOAT4X4 ObjectToWorld;
		DirectX::XMFLOAT4X4 WorldToObject;
	} mObjectBufferData;
	ConstantBuffer* mConstantBuffer;
};

