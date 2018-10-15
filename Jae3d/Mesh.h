#pragma once
#include <vector>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include "d3dx12.h"
#include <DirectXMath.h>

struct Vertex {
	static const int InputElementCount = 2;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;

	Vertex() { }
	Vertex(const DirectX::XMFLOAT3 &position, const DirectX::XMFLOAT3 &normal) : position(position), normal(normal) {}
	Vertex(DirectX::FXMVECTOR position, DirectX::FXMVECTOR normal) {
		XMStoreFloat3(&this->position, position);
		XMStoreFloat3(&this->normal, normal);
	}
};

class Mesh {
public:
	struct ObjectCB {
		DirectX::XMMATRIX ObjectToWorld;
		DirectX::XMMATRIX WorldToObject;
	};

	size_t m_IndexCount;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	~Mesh();
	void Mesh::LoadObj(LPCWSTR file);
	void Mesh::Create(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	void Mesh::UploadData(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** dst,
		ID3D12Resource** intermediate,
		size_t count, size_t stride, const void* data, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	_WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	_WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
};
