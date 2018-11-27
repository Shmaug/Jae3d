#include "Mesh.hpp"

#include "Graphics.hpp"
#include "CommandQueue.hpp"
#include "Camera.hpp"
#include "Util.hpp"

#include "../Common/MeshAsset.hpp"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;


const D3D12_INPUT_ELEMENT_DESC Vertex::InputElements[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

Mesh::Mesh(string name) : MeshAsset(name) {}
Mesh::Mesh(string name, MemoryStream &ms) : MeshAsset(name, ms) {}
Mesh::~Mesh() { ReleaseGpu(); }

void Mesh::UploadData(ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** dst, ID3D12Resource** intermediate,
	size_t count, size_t stride, const void* data) {

	auto device = Graphics::GetDevice();

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(stride * count),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(dst)));

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

void Mesh::LoadCube(float s) {
	vertices = {
		// top
		{ XMFLOAT3( s, s,  s), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(1, 1) }, // 0
		{ XMFLOAT3(-s, s,  s), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(0, 1) }, // 1
		{ XMFLOAT3( s, s, -s), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(1, 0) }, // 2
		{ XMFLOAT3(-s, s, -s), XMFLOAT3(0, 1.0f, 0), XMFLOAT2(0, 0) }, // 3
		
		// bottom
		{ XMFLOAT3( s, -s,  s), XMFLOAT3(0, -1.0f, 0), XMFLOAT2(1, 1) }, // 4
		{ XMFLOAT3( s, -s, -s), XMFLOAT3(0, -1.0f, 0), XMFLOAT2(1, 0) }, // 5
		{ XMFLOAT3(-s, -s,  s), XMFLOAT3(0, -1.0f, 0), XMFLOAT2(0, 1) }, // 6
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(0, -1.0f, 0), XMFLOAT2(0, 0) }, // 7
		
		// right
		{ XMFLOAT3(s,  s,  s), XMFLOAT3(1.0f, 0, 0), XMFLOAT2(1, 1) }, // 8
		{ XMFLOAT3(s, -s,  s), XMFLOAT3(1.0f, 0, 0), XMFLOAT2(1, 0) }, // 9
		{ XMFLOAT3(s,  s, -s), XMFLOAT3(1.0f, 0, 0), XMFLOAT2(0, 1) }, // 10
		{ XMFLOAT3(s, -s, -s), XMFLOAT3(1.0f, 0, 0), XMFLOAT2(0, 0) }, // 11

		// left
		{ XMFLOAT3(-s,  s,  s), XMFLOAT3(-1.0f, 0, 0), XMFLOAT2(1, 1) }, // 12
		{ XMFLOAT3(-s, -s,  s), XMFLOAT3(-1.0f, 0, 0), XMFLOAT2(1, 0) }, // 13
		{ XMFLOAT3(-s,  s, -s), XMFLOAT3(-1.0f, 0, 0), XMFLOAT2(0, 1) }, // 14
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(-1.0f, 0, 0), XMFLOAT2(0, 0) }, // 15

		// forward
		{ XMFLOAT3( s,  s, -s), XMFLOAT3(0, 0, -1.0f), XMFLOAT2(1, 1) }, // 16
		{ XMFLOAT3(-s,  s, -s), XMFLOAT3(0, 0, -1.0f), XMFLOAT2(1, 0) }, // 17
		{ XMFLOAT3( s, -s, -s), XMFLOAT3(0, 0, -1.0f), XMFLOAT2(0, 1) }, // 18
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(0, 0, -1.0f), XMFLOAT2(0, 0) }, // 19

		// back
		{ XMFLOAT3( s,  s, s), XMFLOAT3(0, 0, 1.0f), XMFLOAT2(1, 1) }, // 20
		{ XMFLOAT3( s, -s, s), XMFLOAT3(0, 0, 1.0f), XMFLOAT2(0, 1) }, // 21
		{ XMFLOAT3(-s,  s, s), XMFLOAT3(0, 0, 1.0f), XMFLOAT2(1, 0) }, // 22
		{ XMFLOAT3(-s, -s, s), XMFLOAT3(0, 0, 1.0f), XMFLOAT2(0, 0) }, // 23
	};

	AddTriangle(0, 1, 2);
	AddTriangle(2, 1, 3);
	AddTriangle(4, 5, 6);
	AddTriangle(6, 5, 7);
	AddTriangle(8, 10, 9);
	AddTriangle(9, 10, 11);
	AddTriangle(12, 13, 14);
	AddTriangle(14, 13, 15);
	AddTriangle(16, 17, 18);
	AddTriangle(18, 17, 19);
	AddTriangle(20, 21, 22);
	AddTriangle(22, 21, 23);
}

void Mesh::Create() {
	if (m_Created) return;
	m_Created = true;
	if (vertices.size() < 3 || IndexCount() < 3) return;

	auto device = Graphics::GetDevice();
	auto commandList = Graphics::GetCommandQueue()->GetCommandList();

	m_IndexCount = (UINT)IndexCount();

	ComPtr<ID3D12Resource> ivb;
	UploadData(commandList, &m_VertexBuffer, &ivb, vertices.size(), sizeof(Vertex), vertices.data());
	ComPtr<ID3D12Resource> iib;
	if (Use32BitIndices())
		UploadData(commandList, &m_IndexBuffer, &iib, IndexCount(), sizeof(uint32_t), reinterpret_cast<uint32_t*>(GetIndices32()));
	else
		UploadData(commandList, &m_IndexBuffer, &iib, IndexCount(), sizeof(uint32_t), reinterpret_cast<uint16_t*>(GetIndices16()));

	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.SizeInBytes = (UINT)vertices.size() * sizeof(Vertex);
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);
	
	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	if (Use32BitIndices()) {
		m_IndexBufferView.SizeInBytes = m_IndexCount * sizeof(uint32_t);
		m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	} else {
		m_IndexBufferView.SizeInBytes = m_IndexCount * sizeof(uint16_t);
		m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}

	Graphics::TransitionResource(commandList, m_VertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	Graphics::TransitionResource(commandList, m_IndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	ivb->SetName(L"Vertex Buffer Upload");
	m_VertexBuffer->SetName(L"Vertex Buffer");
	iib->SetName(L"Index Buffer Upload");
	m_IndexBuffer->SetName(L"Index Buffer");

	auto commandQueue = Graphics::GetCommandQueue();
	auto fenceValue = commandQueue->Execute(commandList);
	commandQueue->WaitForFenceValue(fenceValue);
}

void Mesh::ReleaseGpu() {
	m_VertexBuffer.Reset();
	m_IndexBuffer.Reset();
}
void Mesh::Draw(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	commandList->IASetIndexBuffer(&m_IndexBufferView);
	commandList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
}