#pragma once
#include <vector>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include <DirectXMath.h>
#include "d3dx12.hpp"

#include "../Common/MeshAsset.hpp"

class CommandList;

class Mesh : public MeshAsset {
public:
	Mesh(std::string name);
	Mesh(std::string name, MemoryStream &ms);
	~Mesh();

	void LoadCube(float size);
	void Upload();
	void ReleaseGpu();

private:
	friend class CommandList;
	void Draw(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	char* CreateVertexArray(size_t &vsize);

	bool m_DataUploaded = false;

	_WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	_WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	UINT m_IndexCount;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};