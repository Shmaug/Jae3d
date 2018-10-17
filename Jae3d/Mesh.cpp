#include "Mesh.h"

#include "Graphics.h"
#include "CommandQueue.h"
#include "Util.h"

#include <map>
#include <fstream>

using namespace Microsoft::WRL;
using namespace DirectX;

const D3D12_INPUT_ELEMENT_DESC Vertex::InputElements[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

Mesh::~Mesh() {}

void Mesh::UploadData(ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** dst,
	ID3D12Resource** intermediate,
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

void Mesh::LoadObj(LPCSTR path) {
	FILE *file;
	errno_t e = fopen_s(&file, path, "r");
	if (file == NULL) {
		throw std::exception();
		return;
	}

	XMFLOAT3 min(0,0,0);
	XMFLOAT3 max(0,0,0);

	std::vector<XMFLOAT3> tvertices;
	std::vector<XMFLOAT3> tnormals;
	std::vector<XMFLOAT2> tuvs;

	std::map<int, uint32_t> indexMap;

	int ff = 0;

	unsigned int smooth = 0;
	vertices.clear();
	indices.clear();
	while (1) {
		char header[128];
		unsigned int res = fscanf_s(file, "%s", header, 128);
		if (res == EOF) break;
		if (strcmp(header, "v") == 0) {
			// read vertex
			XMFLOAT3 v;
			fscanf_s(file, "%f %f %f\n", &v.x, &v.y, &v.z);
			tvertices.push_back(v);
			max.x = fmax(max.x, v.x);
			max.y = fmax(max.y, v.y);
			max.z = fmax(max.z, v.z);
			min.x = fmin(min.x, v.x);
			min.y = fmin(min.y, v.y);
			min.z = fmin(min.z, v.z);
		} else if (strcmp(header, "vn") == 0) {
			// read normal
			XMFLOAT3 n;
			fscanf_s(file, "%f %f %f\n", &n.x, &n.y, &n.z);
			tnormals.push_back(n);
		} else if (strcmp(header, "vt") == 0) {
			// read tex coord
			XMFLOAT2 t;
			fscanf_s(file, "%f %f\n", &t.x, &t.y);
			tuvs.push_back(t);
		} else if (strcmp(header, "s") == 0) {
			// smoothing?
			fscanf_s(file, "%d\n", &smooth);
		} else if (strcmp(header, "f") == 0) {
			// read face
			unsigned int iv0, in0;
			unsigned int iv1, in1;
			unsigned int iv2, in2;
			fscanf_s(file, "%u//%u %u//%u %u//%u\n", &iv0, &in0, &iv1, &in1, &iv2, &in2);

			iv0--;
			iv1--;
			iv2--;
			in0--;
			in1--;
			in2--;

			unsigned int h0 = (iv0 + in0)*(iv0 + in0 + 1) / 2 + in0;
			unsigned int h1 = (iv1 + in1)*(iv1 + in1 + 1) / 2 + in1;
			unsigned int h2 = (iv2 + in2)*(iv2 + in2 + 1) / 2 + in2;

			if (indexMap.count(h0) == 0) {
				indexMap[h0] = (uint32_t)vertices.size();
				indices.push_back((uint32_t)vertices.size());
				Vertex v(tvertices[iv0], tnormals[in0], XMFLOAT2(0, 0));
				vertices.push_back(v);
			} else
				indices.push_back(indexMap.at(h0));

			if (indexMap.count(h1) == 0) {
				indexMap[h1] = (uint32_t)vertices.size();
				indices.push_back((uint32_t)vertices.size());
				Vertex v(tvertices[iv1], tnormals[in1], XMFLOAT2(0, 0));
				vertices.push_back(v);
			} else
				indices.push_back(indexMap.at(h1));

			if (indexMap.count(h2) == 0) {
				indexMap[h0] = (uint32_t)vertices.size();
				indices.push_back((uint32_t)vertices.size());
				Vertex v(tvertices[iv2], tnormals[in2], XMFLOAT2(0, 0));
				vertices.push_back(v);
			} else
				indices.push_back(indexMap.at(h2));
		}
	}

	fclose(file);

	m_IndexCount = (UINT)indices.size();

	tvertices.clear();
	tnormals.clear();
	tuvs.clear();

	char str[128];
	sprintf_s(str, "Loaded %s, %d vertices, %d faces (%f, %f, %f)\n", path, (int)vertices.size(), (int)indices.size() / 3, max.x - min.x, max.y - min.y, max.z - min.z);
	OutputDebugString(str);

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
	indices = {
		0,  2,  1,  1,  2,  3,
		4,  6,  5,  5,  6,  7,
		8,  9,  10, 10, 9, 11,
		12, 14, 13, 13, 14, 15,
		16, 18, 17, 17, 18, 19,
		20, 22, 21, 21, 22, 23,
	};
	m_IndexCount = (UINT)indices.size();
}

void Mesh::Create() {
	auto device = Graphics::GetDevice();
	auto commandList = Graphics::GetCommandQueue()->GetCommandList();

	ComPtr<ID3D12Resource> ivb;
	UploadData(commandList, &m_VertexBuffer, &ivb, vertices.size(), sizeof(Vertex), vertices.data());
	ComPtr<ID3D12Resource> iib;
	UploadData(commandList, &m_IndexBuffer, &iib, indices.size(), sizeof(uint32_t), indices.data());

	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.SizeInBytes = (UINT)vertices.size() * sizeof(Vertex);
	m_VertexBufferView.StrideInBytes = sizeof(Vertex);
	
	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.SizeInBytes = (UINT)indices.size() * sizeof(uint32_t);
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

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