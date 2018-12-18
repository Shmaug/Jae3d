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
	mSemantics = (MESH_SEMANTIC)ms.Read<uint32_t>();
	VertexCount(ms.Read<uint32_t>());

	for (unsigned int i = 0; i < VertexCount(); i++) {
		mVertices[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(MESH_SEMANTIC_NORMAL)) mNormals[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(MESH_SEMANTIC_TANGENT)) mTangents[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(MESH_SEMANTIC_BINORMAL)) mBinormals[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(MESH_SEMANTIC_COLOR0)) mColor0[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(MESH_SEMANTIC_COLOR1)) mColor1[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(MESH_SEMANTIC_BLENDINDICES)) mBlendIndices[i] = ms.Read<XMUINT4>();
		if (HasSemantic(MESH_SEMANTIC_BLENDWEIGHT)) mBlendWeights[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD0)) mTexcoord0[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD1)) mTexcoord1[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD2)) mTexcoord2[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD3)) mTexcoord3[i] = ms.Read<XMFLOAT4>();
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
		bones.push_back(MeshBone(name, XMFLOAT4X4(data)));
	}
}
Mesh::~Mesh() {
	Clear();
	ReleaseGpu();
}
uint64_t Mesh::TypeId() { return (uint64_t)ASSET_TYPE_MESH; }

void Mesh::WriteData(MemoryStream &ms) {
	ms.Write((uint32_t)mSemantics);
	ms.Write((uint32_t)VertexCount());

	for (unsigned int i = 0; i < VertexCount(); i++) {
		ms.Write(mVertices[i]);
		if (HasSemantic(MESH_SEMANTIC_NORMAL)) ms.Write(mNormals[i]);
		if (HasSemantic(MESH_SEMANTIC_TANGENT)) ms.Write(mTangents[i]);
		if (HasSemantic(MESH_SEMANTIC_BINORMAL)) ms.Write(mBinormals[i]);
		if (HasSemantic(MESH_SEMANTIC_COLOR0)) ms.Write(mColor0[i]);
		if (HasSemantic(MESH_SEMANTIC_COLOR1)) ms.Write(mColor1[i]);
		if (HasSemantic(MESH_SEMANTIC_BLENDINDICES)) ms.Write(mBlendIndices[i]);
		if (HasSemantic(MESH_SEMANTIC_BLENDWEIGHT)) ms.Write(mBlendWeights[i]);
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD0)) ms.Write(mTexcoord0[i]);
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD1)) ms.Write(mTexcoord1[i]);
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD2)) ms.Write(mTexcoord2[i]);
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD3)) ms.Write(mTexcoord3[i]);
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
	if (HasSemantic(MESH_SEMANTIC_NORMAL)) mNormals.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_TANGENT)) mTangents.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_BINORMAL)) mBinormals.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_COLOR0)) mColor0.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_COLOR1)) mColor1.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_BLENDINDICES)) mBlendIndices.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_BLENDWEIGHT)) mBlendWeights.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD0)) mTexcoord0.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD1)) mTexcoord1.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD2)) mTexcoord2.resize(size, shrink);
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD3)) mTexcoord3.resize(size, shrink);
}
unsigned int Mesh::AddVertex(XMFLOAT3 &v) {
	mVertices.push_back(v);
	if (HasSemantic(MESH_SEMANTIC_NORMAL)) mNormals.push_back(DirectX::XMFLOAT3());
	if (HasSemantic(MESH_SEMANTIC_TANGENT)) mTangents.push_back(DirectX::XMFLOAT3());
	if (HasSemantic(MESH_SEMANTIC_BINORMAL)) mBinormals.push_back(DirectX::XMFLOAT3());
	if (HasSemantic(MESH_SEMANTIC_COLOR0)) mColor0.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(MESH_SEMANTIC_COLOR1)) mColor1.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(MESH_SEMANTIC_BLENDWEIGHT)) mBlendWeights.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(MESH_SEMANTIC_BLENDINDICES)) mBlendIndices.push_back(DirectX::XMUINT4());
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD0)) mTexcoord0.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD1)) mTexcoord1.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD2)) mTexcoord2.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD3)) mTexcoord3.push_back(DirectX::XMFLOAT4());
	if (mVertices.size() > 65535) Use32BitIndices(true);
	return (unsigned int)mVertices.size() - 1;
}

void Mesh::LoadCube(float s) {
	HasSemantic(MESH_SEMANTIC_NORMAL, true);
	HasSemantic(MESH_SEMANTIC_TEXCOORD0, true);
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
	HasSemantic(MESH_SEMANTIC_TEXCOORD0, true);
	VertexCount(4);

	XMFLOAT3* v = GetVertices();
	XMFLOAT4* t = GetTexcoords(0);

	int i = 0;
	// top
	v[i] = XMFLOAT3(-s, -s, 0); t[i++] = XMFLOAT4(0, 1, 0, 0);
	v[i] = XMFLOAT3(-s,  s, 0); t[i++] = XMFLOAT4(0, 0, 0, 0);
	v[i] = XMFLOAT3( s,  s, 0); t[i++] = XMFLOAT4(1, 0, 0, 0);
	v[i] = XMFLOAT3( s, -s, 0); t[i++] = XMFLOAT4(1, 1, 0, 0);

	AddTriangle(0, 1, 2);
	AddTriangle(0, 2, 3);
}

inline size_t Write(BYTE* buffer, uint32_t* v) {
	BYTE* t = (BYTE*)v;
	for (int i = 0; i < sizeof(uint32_t); i++)
		buffer[i] = t[i];
	return sizeof(uint32_t);
}
inline size_t Write(BYTE* buffer, float* v) {
	BYTE* t = (BYTE*)v;
	for (int i = 0; i < sizeof(float); i++)
		buffer[i] = t[i];
	return sizeof(float);
}
inline size_t Write(BYTE* buffer, XMFLOAT3* v) {
	size_t l = 0;
	l += Write(buffer + l, &v->x);
	l += Write(buffer + l, &v->y);
	l += Write(buffer + l, &v->z);
	return l;
}
inline size_t Write(BYTE* buffer, XMFLOAT4* v) {
	size_t l = 0;
	l += Write(buffer + l, &v->x);
	l += Write(buffer + l, &v->y);
	l += Write(buffer + l, &v->z);
	l += Write(buffer + l, &v->w);
	return l;
}
inline size_t Write(BYTE* buffer, XMUINT4* v) {
	size_t l = 0;
	l += Write(buffer + l, &v->x);
	l += Write(buffer + l, &v->y);
	l += Write(buffer + l, &v->z);
	l += Write(buffer + l, &v->w);
	return l;
}

size_t Mesh::VertexSize() {
	size_t vertexSize = 0;
	vertexSize += sizeof(float) * 3;
	if (HasSemantic(MESH_SEMANTIC_NORMAL)) vertexSize += sizeof(float) * 3;
	if (HasSemantic(MESH_SEMANTIC_TANGENT)) vertexSize += sizeof(float) * 3;
	if (HasSemantic(MESH_SEMANTIC_BINORMAL)) vertexSize += sizeof(float) * 3;
	if (HasSemantic(MESH_SEMANTIC_COLOR0)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(MESH_SEMANTIC_COLOR1)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(MESH_SEMANTIC_BLENDINDICES)) vertexSize += sizeof(uint32_t) * 4;
	if (HasSemantic(MESH_SEMANTIC_BLENDWEIGHT)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD0)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD1)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD2)) vertexSize += sizeof(float) * 4;
	if (HasSemantic(MESH_SEMANTIC_TEXCOORD3)) vertexSize += sizeof(float) * 4;
	return vertexSize;
}
void Mesh::WriteVertexArray(BYTE* dst) {
	size_t vertexSize = VertexSize();

	for (unsigned int i = 0; i < VertexCount(); i++) {
		BYTE* j = dst + vertexSize * i;

		j += Write(j, &GetVertices()[i]);
		if (HasSemantic(MESH_SEMANTIC_NORMAL)) j += Write(j, &GetNormals()[i]);
		if (HasSemantic(MESH_SEMANTIC_TANGENT)) j += Write(j, &GetTangents()[i]);
		if (HasSemantic(MESH_SEMANTIC_BINORMAL)) j += Write(j, &GetBinormals()[i]);
		if (HasSemantic(MESH_SEMANTIC_COLOR0)) j += Write(j, &GetColors(0)[i]);
		if (HasSemantic(MESH_SEMANTIC_COLOR1)) j += Write(j, &GetColors(1)[i]);
		if (HasSemantic(MESH_SEMANTIC_BLENDINDICES)) j += Write(j, &GetBlendIndices()[i]);
		if (HasSemantic(MESH_SEMANTIC_BLENDWEIGHT)) j += Write(j, &GetBlendWeights()[i]);
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD0)) j += Write(j, &GetTexcoords(0)[i]);
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD1)) j += Write(j, &GetTexcoords(1)[i]);
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD2)) j += Write(j, &GetTexcoords(2)[i]);
		if (HasSemantic(MESH_SEMANTIC_TEXCOORD3)) j += Write(j, &GetTexcoords(3)[i]);
	}
}

void Mesh::UploadStatic() {
	if (VertexCount() < 3 || IndexCount() < 3) return;
	ReleaseGpu();

	// Create vertex array

	auto commandQueue = Graphics::GetCommandQueue();
	auto commandList = commandQueue->GetCommandList(0);

	auto device = Graphics::GetDevice();

	#pragma region vertex buffer
	size_t vsize = VertexSize();

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VertexCount() * vsize),
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
		IID_PPV_ARGS(&mVertexBuffer)));

	ComPtr<ID3D12Resource> vintermediate;
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VertexCount() * vsize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vintermediate)));

	void* vdata;
	CD3DX12_RANGE vreadRange(0, 0);
	vintermediate->Map(0, &vreadRange, &vdata);
	WriteVertexArray(reinterpret_cast<BYTE*>(vdata));
	vintermediate->Unmap(0, &vreadRange);

	commandList->D3DCommandList()->CopyResource(mVertexBuffer.Get(), vintermediate.Get());
	commandList->TransitionResource(mVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = VertexCount() * (UINT)vsize;
	mVertexBufferView.StrideInBytes = (UINT)vsize;
	#pragma endregion

	#pragma region index buffer
	size_t isize = Use32BitIndices() ? sizeof(uint32_t) : sizeof(uint16_t);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(IndexCount() * isize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mIndexBuffer)));

	ComPtr<ID3D12Resource> iintermediate;
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(IndexCount() * isize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&iintermediate)));

	void* idata;
	CD3DX12_RANGE ireadRange(0, 0);
	iintermediate->Map(0, &ireadRange, &idata);
	if (Use32BitIndices())
		memcpy(idata, GetIndices32(), IndexCount() * isize);
	else
		memcpy(idata, GetIndices16(), IndexCount() * isize);
	iintermediate->Unmap(0, &ireadRange);

	commandList->D3DCommandList()->CopyResource(mIndexBuffer.Get(), iintermediate.Get());
	commandList->TransitionResource(mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	mIndexCount = (UINT)IndexCount();
	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.SizeInBytes = mIndexCount * (UINT)isize;
	mIndexBufferView.Format = Use32BitIndices() ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	#pragma endregion

	commandQueue->WaitForFenceValue(commandQueue->Execute(commandList));

	mVertexBuffer->SetName(L"Vertex Buffer");
	mIndexBuffer->SetName(L"Index Buffer");

	mDataUploaded = true;
	mDataMapped = false;
}

void Mesh::UploadDynamic(size_t vsize, size_t isize) {
	ReleaseGpu();

	auto device = Graphics::GetDevice();

#pragma region vertex buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vsize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&mVertexBuffer)));

	CD3DX12_RANGE vreadRange(0, 0);
	mVertexBuffer->Map(0, &vreadRange, &mMappedVertexBuffer);

	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = (UINT)vsize;
	mVertexBufferView.StrideInBytes = (UINT)vsize;
#pragma endregion

#pragma region index buffer
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(isize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&mIndexBuffer)));

	void* idata;
	CD3DX12_RANGE ireadRange(0, 0);
	mIndexBuffer->Map(0, &ireadRange, &idata);

	mIndexCount = 0;
	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.SizeInBytes = (UINT)isize;
	mIndexBufferView.Format = Use32BitIndices() ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
#pragma endregion

	mVertexBuffer->SetName(L"Vertex Buffer");
	mIndexBuffer->SetName(L"Index Buffer");

	mDataUploaded = true;
	mDataMapped = true;
}

void Mesh::ReleaseGpu() {
	if (mDataMapped) {
		CD3DX12_RANGE readRange(0, 0);
		mIndexBuffer->Unmap(0, &readRange);
		mVertexBuffer->Unmap(0, &readRange);
		mDataMapped = false;
	}
	mVertexBuffer.Reset();
	mIndexBuffer.Reset();
	mDataUploaded = false;
}
void Mesh::Draw(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!mDataUploaded) return;
	if (mIndexCount == 0) return;
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	commandList->IASetIndexBuffer(&mIndexBufferView);
	commandList->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
}