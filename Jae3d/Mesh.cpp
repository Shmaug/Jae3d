#include "Mesh.hpp"

#include "Graphics.hpp"
#include "CommandQueue.hpp"
#include "Camera.hpp"

#include "CommandList.hpp"
#include "MemoryStream.hpp"
#include "AssetFile.hpp"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

Mesh::Mesh(jwstring name) : Asset(name), mDataUploaded(false) {}
Mesh::Mesh(jwstring name, MemoryStream &ms) : Asset(name), mDataUploaded(false) {
	mSemantics = (SEMANTIC)ms.Read<uint32_t>();
	VertexCount(ms.Read<uint32_t>());

	for (unsigned int i = 0; i < VertexCount(); i++) {
		mVertices[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(SEMANTIC_NORMAL)) mNormals[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(SEMANTIC_TANGENT)) mTangents[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(SEMANTIC_BINORMAL)) mBinormals[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(SEMANTIC_COLOR0)) mColor0[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_COLOR1)) mColor1[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_BLENDINDICES)) mBlendIndices[i] = ms.Read<XMUINT4>();
		if (HasSemantic(SEMANTIC_BLENDWEIGHT)) mBlendWeights[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_TEXCOORD0)) mTexcoord0[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_TEXCOORD1)) mTexcoord1[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_TEXCOORD2)) mTexcoord2[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_TEXCOORD3)) mTexcoord3[i] = ms.Read<XMFLOAT4>();
	}

	uint32_t indexCount = ms.Read<uint32_t>();
	m32BitIndices = ms.Read<uint8_t>();
	if (m32BitIndices) {
		mIndices32.reserve(indexCount);
		for (uint32_t i = 0; i < indexCount; i++)
			mIndices32.push_back(ms.Read<uint32_t>());
	} else {
		mIndices16.reserve(indexCount);
		for (uint32_t i = 0; i < indexCount; i++)
			mIndices16.push_back(ms.Read<uint16_t>());
	}

	uint32_t bonec = ms.Read<uint32_t>();
	bones.reserve(bonec);
	for (uint32_t i = 0; i < bonec; i++) {
		jwstring name = ms.Read<jwstring>();
		float data[16];
		ms.Read(reinterpret_cast<char*>(data), sizeof(data));
		bones.push_back(Bone(name, XMFLOAT4X4(data)));
	}
}
Mesh::~Mesh() {
	Clear();
	ReleaseGpu();
}
uint64_t Mesh::TypeId() { return (uint64_t)AssetFile::TYPEID_MESH; }

void Mesh::WriteData(MemoryStream &ms) {
	Asset::WriteData(ms);

	ms.Write((uint32_t)mSemantics);
	ms.Write((uint32_t)VertexCount());

	for (unsigned int i = 0; i < VertexCount(); i++) {
		ms.Write(mVertices[i]);
		if (HasSemantic(SEMANTIC_NORMAL)) ms.Write(mNormals[i]);
		if (HasSemantic(SEMANTIC_TANGENT)) ms.Write(mTangents[i]);
		if (HasSemantic(SEMANTIC_BINORMAL)) ms.Write(mBinormals[i]);
		if (HasSemantic(SEMANTIC_COLOR0)) ms.Write(mColor0[i]);
		if (HasSemantic(SEMANTIC_COLOR1)) ms.Write(mColor1[i]);
		if (HasSemantic(SEMANTIC_BLENDINDICES)) ms.Write(mBlendIndices[i]);
		if (HasSemantic(SEMANTIC_BLENDWEIGHT)) ms.Write(mBlendWeights[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD0)) ms.Write(mTexcoord0[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD1)) ms.Write(mTexcoord1[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD2)) ms.Write(mTexcoord2[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD3)) ms.Write(mTexcoord3[i]);
	}

	ms.Write((uint32_t)IndexCount());
	ms.Write((uint8_t)m32BitIndices);
	if (m32BitIndices)
		for (unsigned int i = 0; i < IndexCount(); i++)
			ms.Write(mIndices32[i]);
	else
		for (unsigned int i = 0; i < IndexCount(); i++)
			ms.Write(mIndices16[i]);

	ms.Write((uint32_t)bones.size());
	for (int i = 0; i < bones.size(); i++) {
		ms.WriteString(bones[i].mName);
		float data[16] = {
			bones[i].mBoneToMesh._11, bones[i].mBoneToMesh._12, bones[i].mBoneToMesh._13, bones[i].mBoneToMesh._14,
			bones[i].mBoneToMesh._21, bones[i].mBoneToMesh._22, bones[i].mBoneToMesh._23, bones[i].mBoneToMesh._24,
			bones[i].mBoneToMesh._31, bones[i].mBoneToMesh._32, bones[i].mBoneToMesh._33, bones[i].mBoneToMesh._34,
			bones[i].mBoneToMesh._41, bones[i].mBoneToMesh._42, bones[i].mBoneToMesh._43, bones[i].mBoneToMesh._44
		};
		ms.Write(reinterpret_cast<const char*>(data), sizeof(data));
	}
}

void Mesh::Clear() {
	bones.free();
	mVertices.free();
	mNormals.free();
	mTangents.free();
	mBinormals.free();
	mColor0.free();
	mColor1.free();
	mBlendIndices.free();
	mBlendWeights.free();
	mTexcoord0.free();
	mTexcoord1.free();
	mTexcoord2.free();
	mTexcoord3.free();

	mIndices16.free();
	mIndices32.free();
}

void Mesh::Use32BitIndices(bool v) {
	if (m32BitIndices == v) return;
	m32BitIndices = v;
	if (v) {
		// switch to 32 bit indices
		mIndices32.reserve(mIndices16.size());
		for (int i = 0; i < mIndices16.size(); i++)
			mIndices32.push_back((uint32_t)mIndices16[i]);
		mIndices16.free();
	} else {
		// switch to 16 bit indices
		mIndices16.reserve(mIndices32.size());
		for (int i = 0; i < mIndices32.size(); i++)
			mIndices16.push_back((uint16_t)mIndices32[i]);
		mIndices32.free();
	}
}

void Mesh::VertexCount(unsigned int size, bool shrink) {
	if (size == mVertices.size()) return;

	if (size > 65535 && mVertices.size() <= 65535)
		Use32BitIndices(true);
	else if (size <= 65535 && mVertices.size() > 65535)
		Use32BitIndices(false);

	mVertices.resize(size);
	if (HasSemantic(SEMANTIC_NORMAL)) mNormals.resize(size, shrink);
	if (HasSemantic(SEMANTIC_TANGENT)) mTangents.resize(size, shrink);
	if (HasSemantic(SEMANTIC_BINORMAL)) mBinormals.resize(size, shrink);
	if (HasSemantic(SEMANTIC_COLOR0)) mColor0.resize(size, shrink);
	if (HasSemantic(SEMANTIC_COLOR1)) mColor1.resize(size, shrink);
	if (HasSemantic(SEMANTIC_BLENDINDICES)) mBlendIndices.resize(size, shrink);
	if (HasSemantic(SEMANTIC_BLENDWEIGHT)) mBlendWeights.resize(size, shrink);
	if (HasSemantic(SEMANTIC_TEXCOORD0)) mTexcoord0.resize(size, shrink);
	if (HasSemantic(SEMANTIC_TEXCOORD1)) mTexcoord1.resize(size, shrink);
	if (HasSemantic(SEMANTIC_TEXCOORD2)) mTexcoord2.resize(size, shrink);
	if (HasSemantic(SEMANTIC_TEXCOORD3)) mTexcoord3.resize(size, shrink);
}
unsigned int Mesh::AddVertex(XMFLOAT3 &v) {
	mVertices.push_back(v);
	if (HasSemantic(SEMANTIC_NORMAL)) mNormals.push_back(DirectX::XMFLOAT3());
	if (HasSemantic(SEMANTIC_TANGENT)) mTangents.push_back(DirectX::XMFLOAT3());
	if (HasSemantic(SEMANTIC_BINORMAL)) mBinormals.push_back(DirectX::XMFLOAT3());
	if (HasSemantic(SEMANTIC_COLOR0)) mColor0.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_COLOR1)) mColor1.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_BLENDWEIGHT)) mBlendWeights.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_BLENDINDICES)) mBlendIndices.push_back(DirectX::XMUINT4());
	if (HasSemantic(SEMANTIC_TEXCOORD0)) mTexcoord0.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_TEXCOORD1)) mTexcoord1.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_TEXCOORD2)) mTexcoord2.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_TEXCOORD3)) mTexcoord3.push_back(DirectX::XMFLOAT4());
	if (mVertices.size() > 65535) Use32BitIndices(true);
	return (unsigned int)mVertices.size() - 1;
}

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
void Mesh::LoadQuad(float s) {
	HasSemantic(SEMANTIC_TEXCOORD0, true);
	VertexCount(4);

	XMFLOAT3* v = GetVertices();
	XMFLOAT4* t = GetTexcoords(0);

	int i = 0;
	// top
	v[i] = XMFLOAT3(-s, -s, 0); t[i++] = XMFLOAT4(0, 0, 0, 0);
	v[i] = XMFLOAT3(-s,  s, 0); t[i++] = XMFLOAT4(0, 1, 0, 0);
	v[i] = XMFLOAT3( s,  s, 0); t[i++] = XMFLOAT4(1, 1, 0, 0);
	v[i] = XMFLOAT3( s, -s, 0); t[i++] = XMFLOAT4(1, 0, 0, 0);

	AddTriangle(0, 1, 2);
	AddTriangle(0, 2, 3);
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
	auto commandList = commandQueue->GetCommandList(0);

	ComPtr<ID3D12Resource> ivb;
	Graphics::UploadData(commandList, &mVertexBuffer, &ivb, 1, VertexCount() * vsize, varray);

	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = VertexCount() * (UINT)vsize;
	mVertexBufferView.StrideInBytes = (UINT)vsize;

	ComPtr<ID3D12Resource> iib;
	if (Use32BitIndices())
		Graphics::UploadData(commandList, &mIndexBuffer, &iib, IndexCount(), sizeof(uint32_t), GetIndices32());
	else
		Graphics::UploadData(commandList, &mIndexBuffer, &iib, IndexCount(), sizeof(uint16_t), GetIndices16());

	mIndexCount = (UINT)IndexCount();
	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	if (Use32BitIndices()) {
		mIndexBufferView.SizeInBytes = mIndexCount * sizeof(uint32_t);
		mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	} else {
		mIndexBufferView.SizeInBytes = mIndexCount * sizeof(uint16_t);
		mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}

	commandList->TransitionResource(mVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commandList->TransitionResource(mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	mVertexBuffer->SetName(L"Vertex Buffer");
	mIndexBuffer->SetName(L"Index Buffer");

	commandQueue->WaitForFenceValue(commandQueue->Execute(commandList));

	delete[] varray;
	mDataUploaded = true;
}

void Mesh::ReleaseGpu() {
	mVertexBuffer.Reset();
	mIndexBuffer.Reset();
	mDataUploaded = false;
}
void Mesh::Draw(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!mDataUploaded) return;
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	commandList->IASetIndexBuffer(&mIndexBufferView);
	commandList->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
}