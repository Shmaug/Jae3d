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
protected:
	bool UpdateTransform();

public:
	MeshRenderer();
	MeshRenderer(jstring name);
	~MeshRenderer();

	void Draw(std::shared_ptr<CommandList> commandList);

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

