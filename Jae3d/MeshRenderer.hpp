#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>

#include "Object.hpp"
#include "Mesh.hpp"

class MeshRenderer : public Object {
protected:
	bool UpdateTransform();

public:
	std::shared_ptr<Mesh> m_Mesh;
	MeshRenderer(std::string name);
	~MeshRenderer();

	void Create();
	void Release();
	D3D12_GPU_VIRTUAL_ADDRESS GetCBuffer() { if (m_TransformDirty) UpdateTransform(); return m_CBuffer->GetGPUVirtualAddress(); }
	void Draw(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	struct ObjectBuffer {
	public:
		DirectX::XMFLOAT4X4 ObjectToWorld;
		DirectX::XMFLOAT4X4 WorldToObject;
	} m_ObjectBufferData;
	_WRL::ComPtr<ID3D12Resource> m_CBuffer;
	UINT8* m_MappedCBuffer;
};

