#pragma once

#include <memory>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>

#include "Object.hpp"
#include "Mesh.hpp"

class MeshRenderer : public Object {
protected:
	bool UpdateTransform();

public:
	MeshRenderer(std::string name);
	~MeshRenderer();

	void SetMesh(std::shared_ptr<Mesh> m) { if (m) m->Create(); m_Mesh = m; }
	std::shared_ptr<Mesh> GetMesh() const { return m_Mesh; }

	void Create();
	void Release();
	D3D12_GPU_VIRTUAL_ADDRESS GetCBuffer() { if (m_TransformDirty) UpdateTransform(); return m_CBuffer->GetGPUVirtualAddress(); }
	void Draw(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	std::shared_ptr<Mesh> m_Mesh;
	struct ObjectBuffer {
	public:
		DirectX::XMFLOAT4X4 ObjectToWorld;
		DirectX::XMFLOAT4X4 WorldToObject;
	} m_ObjectBufferData;
	_WRL::ComPtr<ID3D12Resource> m_CBuffer;
	UINT8* m_MappedCBuffer;
};

