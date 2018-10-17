#pragma once
#include <vector>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include "d3dx12.h"
#include <DirectXMath.h>

struct Vertex {
	static const int InputElementCount = 3;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;

	Vertex() { }
	Vertex(const DirectX::XMFLOAT3 &position, const DirectX::XMFLOAT3 &normal, const DirectX::XMFLOAT2 &uv) : position(position), normal(normal), uv(uv) {}
	Vertex(DirectX::FXMVECTOR position, DirectX::FXMVECTOR normal, DirectX::FXMVECTOR uv) {
		XMStoreFloat3(&this->position, position);
		XMStoreFloat3(&this->normal, normal);
		XMStoreFloat2(&this->uv, uv);
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
	void Mesh::LoadObj(LPCSTR file);
	void Mesh::LoadCube(float size);
	void Mesh::Create();

private:
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	void Mesh::UploadData(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** dst,
		ID3D12Resource** intermediate,
		size_t count, size_t stride, const void* data);

	_WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	_WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
};
