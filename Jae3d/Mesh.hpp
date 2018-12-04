#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <DirectXMath.h>

#include "../Common/d3dx12.hpp"
#include "../Common/MeshAsset.hpp"

class CommandList;

class Mesh : public MeshAsset {
public:
	Mesh(jstring name);
	Mesh(jstring name, MemoryStream &ms);
	~Mesh();

	void LoadCube(float size);
	void Upload();
	void ReleaseGpu();

private:
	friend class CommandList;
	void Draw(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	char* CreateVertexArray(size_t &vsize);

	bool mDataUploaded = false;

	_WRL::ComPtr<ID3D12Resource> mVertexBuffer;
	_WRL::ComPtr<ID3D12Resource> mIndexBuffer;
	UINT mIndexCount;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
};