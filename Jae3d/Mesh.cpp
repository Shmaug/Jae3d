#include "Mesh.h"

#include "Graphics.h"
#include "CommandQueue.h"
#include "Util.h"

using namespace Microsoft::WRL;
using namespace DirectX;


const D3D12_INPUT_ELEMENT_DESC Vertex::InputElements[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

Mesh::~Mesh() {}

void Mesh::UploadData(ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** dst,
	ID3D12Resource** intermediate,
	size_t count, size_t stride, const void* data, D3D12_RESOURCE_FLAGS flags) {

	auto device = Graphics::GetDevice();

	ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(stride * count, D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(dst)));

	if (data) {
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(count * stride),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(intermediate)));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = data;
		subresourceData.RowPitch = count * stride;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		UpdateSubresources(commandList.Get(), *dst, *intermediate, 0, 0, 1, &subresourceData);
	}
}

void Mesh::LoadObj(LPCWSTR file) {
	float s = .5f;
	vertices = {
		// top
		{ XMFLOAT3( s, s,  s), XMFLOAT3(0, 1.0f, 0) }, // 0
		{ XMFLOAT3(-s, s,  s), XMFLOAT3(0, 1.0f, 0) }, // 1
		{ XMFLOAT3( s, s, -s), XMFLOAT3(0, 1.0f, 0) }, // 2
		{ XMFLOAT3(-s, s, -s), XMFLOAT3(0, 1.0f, 0) }, // 3
		
		// bottom
		{ XMFLOAT3( s, s,  s), XMFLOAT3(0, -1.0f, 0) }, // 4
		{ XMFLOAT3( s, s, -s), XMFLOAT3(0, -1.0f, 0) }, // 5
		{ XMFLOAT3(-s, s,  s), XMFLOAT3(0, -1.0f, 0) }, // 6
		{ XMFLOAT3(-s, s, -s), XMFLOAT3(0, -1.0f, 0) }, // 7
		
		// right
		{ XMFLOAT3(s,  s,  s), XMFLOAT3(1.0f, 0, 0) }, // 8
		{ XMFLOAT3(s,  s, -s), XMFLOAT3(1.0f, 0, 0) }, // 9
		{ XMFLOAT3(s, -s,  s), XMFLOAT3(1.0f, 0, 0) }, // 10
		{ XMFLOAT3(s, -s, -s), XMFLOAT3(1.0f, 0, 0) }, // 11

		// left
		{ XMFLOAT3(-s,  s,  s), XMFLOAT3(-1.0f, 0, 0) }, // 12
		{ XMFLOAT3(-s, -s,  s), XMFLOAT3(-1.0f, 0, 0) }, // 13
		{ XMFLOAT3(-s,  s, -s), XMFLOAT3(-1.0f, 0, 0) }, // 14
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(-1.0f, 0, 0) }, // 15

		// forward
		{ XMFLOAT3( s,  s, -s), XMFLOAT3(0, 0, -1.0f) }, // 16
		{ XMFLOAT3( s, -s, -s), XMFLOAT3(0, 0, -1.0f) }, // 17
		{ XMFLOAT3(-s,  s, -s), XMFLOAT3(0, 0, -1.0f) }, // 18
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(0, 0, -1.0f) }, // 19

		// back
		{ XMFLOAT3( s,  s, s), XMFLOAT3(0, 0, 1.0f) }, // 20
		{ XMFLOAT3(-s,  s, s), XMFLOAT3(0, 0, 1.0f) }, // 21
		{ XMFLOAT3( s, -s, s), XMFLOAT3(0, 0, 1.0f) }, // 22
		{ XMFLOAT3(-s, -s, s), XMFLOAT3(0, 0, 1.0f) }, // 23
	};

	indices = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23,
	};
	m_IndexCount = indices.size();
}

void Mesh::Create(ComPtr<ID3D12GraphicsCommandList2> g) {
	auto device = Graphics::GetDevice();
	auto commandList = Graphics::GetCommandQueue()->GetCommandList();

	ComPtr<ID3D12Resource> ivb;
	UploadData(commandList, &m_VertexBuffer, &ivb, vertices.size(), sizeof(Vertex), vertices.data());

	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.SizeInBytes = (UINT)vertices.size() * sizeof(Vertex);
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);

	ComPtr<ID3D12Resource> iib;
	UploadData(commandList, &m_IndexBuffer, &iib, indices.size(), sizeof(uint16_t), indices.data());

	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.SizeInBytes = (UINT)indices.size() * sizeof(uint16_t);
	m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;

	auto commandQueue = Graphics::GetCommandQueue();
	auto fenceValue = commandQueue->Execute(commandList);
	commandQueue->WaitForFenceValue(fenceValue);
}