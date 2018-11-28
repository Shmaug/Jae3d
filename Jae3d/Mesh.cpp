#include "Mesh.hpp"

#include "Graphics.hpp"
#include "CommandQueue.hpp"
#include "Camera.hpp"
#include "Util.hpp"

#include "CommandList.hpp"

#include "../Common/MeshAsset.hpp"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

Mesh::Mesh(string name) : MeshAsset(name), m_DataUploaded(false) {}
Mesh::Mesh(string name, MemoryStream &ms) : MeshAsset(name, ms), m_DataUploaded(false) {}
Mesh::~Mesh() { ReleaseGpu(); }

void Mesh::LoadCube(float s) {
	HasSemantic(SEMANTIC_NORMAL, true);
	HasSemantic(SEMANTIC_TEXCOORD0, true);
	VertexCount(24);

	XMFLOAT3* v = GetVertices();
	XMFLOAT3* n = GetNormals();
	XMFLOAT4* t = GetTexcoords(0);

	int i = 0;
	// top
	v[i] = XMFLOAT3( s, s,  s); n[i] = XMFLOAT3(0, 1.0f, 0); t[i++] = XMFLOAT4(1, 1, 0, 0); // 0
	v[i] = XMFLOAT3(-s, s,  s); n[i] = XMFLOAT3(0, 1.0f, 0); t[i++] = XMFLOAT4(0, 1, 0, 0); // 1
	v[i] = XMFLOAT3( s, s, -s); n[i] = XMFLOAT3(0, 1.0f, 0); t[i++] = XMFLOAT4(1, 0, 0, 0); // 2
	v[i] = XMFLOAT3(-s, s, -s); n[i] = XMFLOAT3(0, 1.0f, 0); t[i++] = XMFLOAT4(0, 0, 0, 0); // 3
		
	// bottom
	v[i] = XMFLOAT3( s, -s,  s); n[i] = XMFLOAT3(0, -1.0f, 0); t[i++] = XMFLOAT4(1, 1, 0, 0); // 4
	v[i] = XMFLOAT3( s, -s, -s); n[i] = XMFLOAT3(0, -1.0f, 0); t[i++] = XMFLOAT4(1, 0, 0, 0); // 5
	v[i] = XMFLOAT3(-s, -s,  s); n[i] = XMFLOAT3(0, -1.0f, 0); t[i++] = XMFLOAT4(0, 1, 0, 0); // 6
	v[i] = XMFLOAT3(-s, -s, -s); n[i] = XMFLOAT3(0, -1.0f, 0); t[i++] = XMFLOAT4(0, 0, 0, 0); // 7
		
	// right
	v[i] = XMFLOAT3(s,  s,  s); n[i] = XMFLOAT3(1.0f, 0, 0); t[i++] = XMFLOAT4(1, 1, 0, 0); // 8
	v[i] = XMFLOAT3(s, -s,  s); n[i] = XMFLOAT3(1.0f, 0, 0); t[i++] = XMFLOAT4(1, 0, 0, 0); // 9
	v[i] = XMFLOAT3(s,  s, -s); n[i] = XMFLOAT3(1.0f, 0, 0); t[i++] = XMFLOAT4(0, 1, 0, 0); // 10
	v[i] = XMFLOAT3(s, -s, -s); n[i] = XMFLOAT3(1.0f, 0, 0); t[i++] = XMFLOAT4(0, 0, 0, 0); // 11

	// left
	v[i] = XMFLOAT3(-s,  s,  s); n[i] = XMFLOAT3(-1.0f, 0, 0); t[i++] = XMFLOAT4(1, 1, 0, 0); // 12
	v[i] = XMFLOAT3(-s, -s,  s); n[i] = XMFLOAT3(-1.0f, 0, 0); t[i++] = XMFLOAT4(1, 0, 0, 0); // 13
	v[i] = XMFLOAT3(-s,  s, -s); n[i] = XMFLOAT3(-1.0f, 0, 0); t[i++] = XMFLOAT4(0, 1, 0, 0); // 14
	v[i] = XMFLOAT3(-s, -s, -s); n[i] = XMFLOAT3(-1.0f, 0, 0); t[i++] = XMFLOAT4(0, 0, 0, 0); // 15

	// forward
	v[i] = XMFLOAT3( s,  s, -s); n[i] = XMFLOAT3(0, 0, -1.0f); t[i++] = XMFLOAT4(1, 1, 0, 0); // 16
	v[i] = XMFLOAT3(-s,  s, -s); n[i] = XMFLOAT3(0, 0, -1.0f); t[i++] = XMFLOAT4(1, 0, 0, 0); // 17
	v[i] = XMFLOAT3( s, -s, -s); n[i] = XMFLOAT3(0, 0, -1.0f); t[i++] = XMFLOAT4(0, 1, 0, 0); // 18
	v[i] = XMFLOAT3(-s, -s, -s); n[i] = XMFLOAT3(0, 0, -1.0f); t[i++] = XMFLOAT4(0, 0, 0, 0); // 19

	// back
	v[i] = XMFLOAT3( s,  s, s); n[i] = XMFLOAT3(0, 0, 1.0f); t[i++] = XMFLOAT4(1, 1, 0, 0); // 20
	v[i] = XMFLOAT3( s, -s, s); n[i] = XMFLOAT3(0, 0, 1.0f); t[i++] = XMFLOAT4(0, 1, 0, 0); // 21
	v[i] = XMFLOAT3(-s,  s, s); n[i] = XMFLOAT3(0, 0, 1.0f); t[i++] = XMFLOAT4(1, 0, 0, 0); // 22
	v[i] = XMFLOAT3(-s, -s, s); n[i] = XMFLOAT3(0, 0, 1.0f); t[i++] = XMFLOAT4(0, 0, 0, 0); // 23

	AddTriangle(1,  0,  2);
	AddTriangle(1,  2,  3);
	AddTriangle(5,  4,  6);
	AddTriangle(5,  6,  7);
	AddTriangle(10, 8,  9);
	AddTriangle(10, 9,  11);
	AddTriangle(13, 12, 14);
	AddTriangle(13, 14, 15);
	AddTriangle(17, 16, 18);
	AddTriangle(17, 18, 19);
	AddTriangle(21, 20, 22);
	AddTriangle(21, 22, 23);
}

inline size_t Write(char* buffer, uint32_t* v) {
	char* t = (char*)v;
	for (int i = 0; i < sizeof(uint32_t); i++)
		buffer[i] = t[i];
	return sizeof(uint32_t);
}
inline size_t Write(char* buffer, float* v) {
	char* t = (char*)v;
	for (int i = 0; i < sizeof(float); i++)
		buffer[i] = t[i];
	return sizeof(float);
}
inline size_t Write(char* buffer, XMFLOAT3* v) {
	size_t l = 0;
	l += Write(buffer + l, &v->x);
	l += Write(buffer + l, &v->y);
	l += Write(buffer + l, &v->z);
	return l;
}
inline size_t Write(char* buffer, XMFLOAT4* v) {
	size_t l = 0;
	l += Write(buffer + l, &v->x);
	l += Write(buffer + l, &v->y);
	l += Write(buffer + l, &v->z);
	l += Write(buffer + l, &v->w);
	return l;
}
inline size_t Write(char* buffer, XMUINT4* v) {
	size_t l = 0;
	l += Write(buffer + l, &v->x);
	l += Write(buffer + l, &v->y);
	l += Write(buffer + l, &v->z);
	l += Write(buffer + l, &v->w);
	return l;
}

char* Mesh::CreateVertexArray(size_t &vertexSize) {
	vertexSize = 0;
	vertexSize += sizeof(float) * 3;
	if (HasSemantic(SEMANTIC_NORMAL)) vertexSize += sizeof(float) * 3;
	if (HasSemantic(SEMANTIC_TANGENT)) vertexSize += sizeof(float) * 3;
	if (HasSemantic(SEMANTIC_BINORMAL)) vertexSize += sizeof(float) * 3;
	if (HasSemantic(SEMANTIC_COLOR0)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(SEMANTIC_COLOR1)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(SEMANTIC_BLENDINDICES)) vertexSize += sizeof(uint32_t) * 4;
	if (HasSemantic(SEMANTIC_BLENDWEIGHT)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(SEMANTIC_TEXCOORD0)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(SEMANTIC_TEXCOORD1)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(SEMANTIC_TEXCOORD2)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(SEMANTIC_TEXCOORD3)) vertexSize += sizeof(float) * 4;

	char* buffer = new char[VertexCount() * vertexSize];

	for (unsigned int i = 0; i < VertexCount(); i++) {
		char* j = buffer + vertexSize * i;

		j += Write(j, &GetVertices()[i]);
		if (HasSemantic(SEMANTIC_NORMAL)) j += Write(j, &GetNormals()[i]);
		if (HasSemantic(SEMANTIC_TANGENT)) j += Write(j, &GetTangents()[i]);
		if (HasSemantic(SEMANTIC_BINORMAL)) j += Write(j, &GetBinormals()[i]);
		if (HasSemantic(SEMANTIC_COLOR0)) j += Write(j, &GetColors(0)[i]);
		if (HasSemantic(SEMANTIC_COLOR1)) j += Write(j, &GetColors(1)[i]);
		if (HasSemantic(SEMANTIC_BLENDINDICES)) j += Write(j, &GetBlendIndices()[i]);
		if (HasSemantic(SEMANTIC_BLENDWEIGHT)) j += Write(j, &GetBlendWeights()[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD0)) j += Write(j, &GetTexcoords(0)[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD1)) j += Write(j, &GetTexcoords(1)[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD2)) j += Write(j, &GetTexcoords(2)[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD3)) j += Write(j, &GetTexcoords(3)[i]);
	}

	return buffer;
}

void Mesh::Upload() {
	if (VertexCount() < 3 || IndexCount() < 3) return;
	ReleaseGpu();

	// Create vertex array
	size_t vsize;
	char* varray = CreateVertexArray(vsize);

	auto commandQueue = Graphics::GetCommandQueue();
	auto commandList = commandQueue->GetCommandList();

	ComPtr<ID3D12Resource> ivb;
	Graphics::UploadData(commandList, &m_VertexBuffer, &ivb, VertexCount(), vsize, varray);
	ivb->SetName(L"Vertex Buffer Upload");

	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.SizeInBytes = VertexCount() * (UINT)vsize;
	m_VertexBufferView.StrideInBytes = (UINT)vsize;

	ComPtr<ID3D12Resource> iib;
	if (Use32BitIndices())
		Graphics::UploadData(commandList, &m_IndexBuffer, &iib, IndexCount(), sizeof(uint32_t), GetIndices32());
	else
		Graphics::UploadData(commandList, &m_IndexBuffer, &iib, IndexCount(), sizeof(uint32_t), GetIndices16());
	iib->SetName(L"Index Buffer Upload");

	m_IndexCount = (UINT)IndexCount();
	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	if (Use32BitIndices()) {
		m_IndexBufferView.SizeInBytes = m_IndexCount * sizeof(uint32_t);
		m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	} else {
		m_IndexBufferView.SizeInBytes = m_IndexCount * sizeof(uint16_t);
		m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}

	commandList->TransitionResource(m_VertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commandList->TransitionResource(m_IndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	m_VertexBuffer->SetName(L"Vertex Buffer");
	m_IndexBuffer->SetName(L"Index Buffer");

	commandQueue->WaitForFenceValue(commandQueue->Execute(commandList));

	delete[] varray;
	m_DataUploaded = true;
}

void Mesh::ReleaseGpu() {
	m_VertexBuffer.Reset();
	m_IndexBuffer.Reset();
	m_DataUploaded = false;
}
void Mesh::Draw(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!m_DataUploaded) return;
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	commandList->IASetIndexBuffer(&m_IndexBufferView);
	commandList->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
}