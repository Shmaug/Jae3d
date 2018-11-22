#pragma once
#include <vector>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include <DirectXMath.h>
#include "d3dx12.hpp"
#include "Asset.hpp"

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

class Mesh : public Asset {
public:
	struct Bone {
		std::string m_Name;
		DirectX::XMFLOAT4X4 m_BoneToMesh;
	};

	Mesh(std::string name);
	~Mesh();

	void LoadCube(float size);
	void Create();
	void Release();

	void Draw(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	std::vector<Bone> bones;

	std::vector<DirectX::XMFLOAT3> m_Vertices;
	std::vector<DirectX::XMFLOAT3> m_Normals;
	std::vector<DirectX::XMFLOAT3> m_Tangents;
	std::vector<DirectX::XMFLOAT3> m_Bitangents;
	std::vector<DirectX::XMFLOAT4> m_Colors;
	std::vector<DirectX::XMINT4> m_BlendIndices;
	std::vector<DirectX::XMFLOAT4> m_BlendWeights;
	std::vector<DirectX::XMFLOAT3> m_Texcoords[8];
	std::vector<int> m_Indices;

	std::vector<Vertex> vertices;

	void UploadData(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** dst,
		ID3D12Resource** intermediate,
		size_t count, size_t stride, const void* data);

	_WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	_WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	UINT m_IndexCount;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};