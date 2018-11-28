#pragma once

#include <memory>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>

#include "Object.hpp"

class CommandList;
class Mesh;
class Material;

class MeshRenderer : public Object {
protected:
	bool UpdateTransform();

public:
	MeshRenderer(std::string name);
	~MeshRenderer();

	void Upload();
	void Release();
	D3D12_GPU_VIRTUAL_ADDRESS GetCBuffer() { if (m_TransformDirty) UpdateTransform(); return m_CBuffer->GetGPUVirtualAddress(); }
	void Draw(std::shared_ptr<CommandList> commandList);

	std::shared_ptr<Material> m_Material;
	std::shared_ptr<Mesh> m_Mesh;

private:
	struct ObjectBuffer {
	public:
		DirectX::XMFLOAT4X4 ObjectToWorld;
		DirectX::XMFLOAT4X4 WorldToObject;
	} m_ObjectBufferData;
	_WRL::ComPtr<ID3D12Resource> m_CBuffer;
	UINT8* m_MappedCBuffer;
};

