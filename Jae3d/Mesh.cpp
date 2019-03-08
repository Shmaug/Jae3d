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

Mesh::Mesh(const jwstring& name) : Asset(name), mDataUploaded(false) {}
Mesh::Mesh(const jwstring& name, MemoryStream &ms) : Asset(name, ms), mDataUploaded(false) {
	mSemantics = (MESH_SEMANTIC)ms.Read<uint32_t>();
	VertexCount(ms.Read<uint32_t>());

	XMFLOAT3 min;
	XMFLOAT3 max;

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
		if (i == 0) {
			min = mVertices[i];
			max = mVertices[i];
		} else {
			min.x = fmin(mVertices[i].x, min.x);
			min.y = fmin(mVertices[i].y, min.y);
			min.z = fmin(mVertices[i].z, min.z);
			max.x = fmax(mVertices[i].x, max.x);
			max.y = fmax(mVertices[i].y, max.y);
			max.z = fmax(mVertices[i].z, max.z);
		}
	}

	mBounds = BoundingBox(
		XMFLOAT3((min.x + max.x) * .5f, (min.y + max.y) * .5f, (min.z + max.z) * .5f),
		XMFLOAT3(max.x - min.x, max.y - min.y, max.z - min.z));

	m32BitIndices = ms.Read<uint8_t>();

	uint32_t submeshCount = ms.Read<uint32_t>();
	mSubmeshes.reserve(submeshCount);
	for (unsigned int s = 0; s < submeshCount; s++) {
		mSubmeshes.push_back({});
		uint32_t indexCount = ms.Read<uint32_t>();
		if (m32BitIndices) {
			mSubmeshes[s].mIndices32.reserve(indexCount);
			for (uint32_t i = 0; i < indexCount; i++)
				mSubmeshes[s].mIndices32.push_back(ms.Read<uint32_t>());
		} else {
			mSubmeshes[s].mIndices16.reserve(indexCount);
			for (uint32_t i = 0; i < indexCount; i++)
				mSubmeshes[s].mIndices16.push_back(ms.Read<uint16_t>());
		}
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

	ms.Write((uint8_t)m32BitIndices);

	ms.Write((uint32_t)mSubmeshes.size());
	for (unsigned int s = 0; s < mSubmeshes.size(); s++) {
		ms.Write((uint32_t)IndexCount(s));
		if (m32BitIndices)
			for (unsigned int i = 0; i < IndexCount(s); i++)
				ms.Write(mSubmeshes[s].mIndices32[i]);
		else
			for (unsigned int i = 0; i < IndexCount(s); i++)
				ms.Write(mSubmeshes[s].mIndices16[i]);
	}

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

	mSubmeshes.free();
}

void Mesh::Use32BitIndices(bool v) {
	if (m32BitIndices == v) return;
	m32BitIndices = v;
	if (v) {
		// switch to 32 bit indices
		for (unsigned int s = 0; s < mSubmeshes.size(); s++) {
			Submesh& sm = mSubmeshes[s];
			sm.mIndices32.reserve(sm.mIndices16.size());
			for (int i = 0; i < sm.mIndices16.size(); i++)
				sm.mIndices32.push_back((uint32_t)sm.mIndices16[i]);
			sm.mIndices16.free();
		}
	} else {
		// switch to 16 bit indices
		for (unsigned int s = 0; s < mSubmeshes.size(); s++) {
			Submesh& sm = mSubmeshes[s];
			sm.mIndices16.reserve(sm.mIndices32.size());
			for (int i = 0; i < sm.mIndices32.size(); i++)
				sm.mIndices16.push_back((uint16_t)sm.mIndices32[i]);
			sm.mIndices32.free();
		}
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
unsigned int Mesh::AddVertex(const XMFLOAT3 &v) {
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

void Mesh::AddIndices(unsigned int count, const unsigned int* indices, unsigned int submesh) {
	while (submesh >= mSubmeshes.size())
		mSubmeshes.push_back({});

	if (m32BitIndices) {
		mSubmeshes[submesh].mIndices32.reserve(mSubmeshes[submesh].mIndices32.size() + count);
		for (unsigned int i = 0; i < count; i++)
			mSubmeshes[submesh].mIndices32.push_back((uint32_t)indices[i]);
	} else {
		mSubmeshes[submesh].mIndices16.reserve(mSubmeshes[submesh].mIndices16.size() + count);
		for (unsigned int i = 0; i < count; i++)
			mSubmeshes[submesh].mIndices16.push_back((uint16_t)indices[i]);
	}
}
void Mesh::AddTriangle(uint32_t i0, uint32_t i1, uint32_t i2, unsigned int submesh){
	while (submesh >= mSubmeshes.size())
		mSubmeshes.push_back({});

	if (m32BitIndices) {
		mSubmeshes[submesh].mIndices32.push_back(i0);
		mSubmeshes[submesh].mIndices32.push_back(i1);
		mSubmeshes[submesh].mIndices32.push_back(i2);
	} else {
		mSubmeshes[submesh].mIndices16.push_back((uint16_t)i0);
		mSubmeshes[submesh].mIndices16.push_back((uint16_t)i1);
		mSubmeshes[submesh].mIndices16.push_back((uint16_t)i2);
	}
}

void Mesh::HasSemantic(MESH_SEMANTIC s, bool v) {
	if (s == MESH_SEMANTIC_POSITION || HasSemantic(s) == v) return;
	if (v) {
		mSemantics = (MESH_SEMANTIC)(mSemantics | s);
		switch (s) {
		case MESH_SEMANTIC_NORMAL:
			mNormals.resize(VertexCount());
			break;
		case MESH_SEMANTIC_TANGENT:
			mTangents.resize(VertexCount());
			break;
		case MESH_SEMANTIC_BINORMAL:
			mBinormals.resize(VertexCount());
			break;
		case MESH_SEMANTIC_COLOR0:
			mColor0.resize(VertexCount());
			break;
		case MESH_SEMANTIC_COLOR1:
			mColor1.resize(VertexCount());
			break;
		case MESH_SEMANTIC_BLENDINDICES:
			mBlendIndices.resize(VertexCount());
			break;
		case MESH_SEMANTIC_BLENDWEIGHT:
			mBlendWeights.resize(VertexCount());
			break;
		case MESH_SEMANTIC_TEXCOORD0:
			mTexcoord0.resize(VertexCount());
			break;
		case MESH_SEMANTIC_TEXCOORD1:
			mTexcoord1.resize(VertexCount());
			break;
		case MESH_SEMANTIC_TEXCOORD2:
			mTexcoord2.resize(VertexCount());
			break;
		case MESH_SEMANTIC_TEXCOORD3:
			mTexcoord3.resize(VertexCount());
			break;
		}
	} else {
		mSemantics = (MESH_SEMANTIC)(mSemantics & ~s);
		switch (s) {
		case MESH_SEMANTIC_NORMAL:
			mNormals.free();
			break;
		case MESH_SEMANTIC_TANGENT:
			mTangents.free();
			break;
		case MESH_SEMANTIC_BINORMAL:
			mBinormals.free();
			break;
		case MESH_SEMANTIC_COLOR0:
			mColor0.free();
			break;
		case MESH_SEMANTIC_COLOR1:
			mColor1.free();
			break;
		case MESH_SEMANTIC_BLENDINDICES:
			mBlendIndices.free();
			break;
		case MESH_SEMANTIC_BLENDWEIGHT:
			mBlendWeights.free();
			break;
		case MESH_SEMANTIC_TEXCOORD0:
			mTexcoord0.free();
			break;
		case MESH_SEMANTIC_TEXCOORD1:
			mTexcoord1.free();
			break;
		case MESH_SEMANTIC_TEXCOORD2:
			mTexcoord2.free();
			break;
		case MESH_SEMANTIC_TEXCOORD3:
			mTexcoord3.free();
			break;
		}
	}
}

void Mesh::LoadCube(float s, unsigned int submesh) {
	HasSemantic(MESH_SEMANTIC_TEXCOORD0, true);
	HasSemantic(MESH_SEMANTIC_NORMAL, true);
	HasSemantic(MESH_SEMANTIC_TANGENT, true);
	int si = VertexCount();
	VertexCount(VertexCount() + 24);

	XMFLOAT3* v = GetVertices();
	XMFLOAT3* n = GetNormals();
	XMFLOAT4* t = GetTexcoords(0);
	XMFLOAT3* g = GetTangents();

	int i = si;
	// top
	v[i] = XMFLOAT3( s, s,  s); n[i] = XMFLOAT3(0, 1.0f, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(1, 1, 0, 0); // 0
	v[i] = XMFLOAT3(-s, s,  s); n[i] = XMFLOAT3(0, 1.0f, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(0, 1, 0, 0); // 1
	v[i] = XMFLOAT3( s, s, -s); n[i] = XMFLOAT3(0, 1.0f, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(1, 0, 0, 0); // 2
	v[i] = XMFLOAT3(-s, s, -s); n[i] = XMFLOAT3(0, 1.0f, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(0, 0, 0, 0); // 3
		
	// bottom
	v[i] = XMFLOAT3( s, -s,  s); n[i] = XMFLOAT3(0, -1.0f, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(1, 1, 0, 0); // 4
	v[i] = XMFLOAT3( s, -s, -s); n[i] = XMFLOAT3(0, -1.0f, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(1, 0, 0, 0); // 5
	v[i] = XMFLOAT3(-s, -s,  s); n[i] = XMFLOAT3(0, -1.0f, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(0, 1, 0, 0); // 6
	v[i] = XMFLOAT3(-s, -s, -s); n[i] = XMFLOAT3(0, -1.0f, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(0, 0, 0, 0); // 7
		
	// right
	v[i] = XMFLOAT3(s,  s,  s); n[i] = XMFLOAT3(1.0f, 0, 0); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(1, 1, 0, 0); // 8
	v[i] = XMFLOAT3(s, -s,  s); n[i] = XMFLOAT3(1.0f, 0, 0); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(1, 0, 0, 0); // 9
	v[i] = XMFLOAT3(s,  s, -s); n[i] = XMFLOAT3(1.0f, 0, 0); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(0, 1, 0, 0); // 10
	v[i] = XMFLOAT3(s, -s, -s); n[i] = XMFLOAT3(1.0f, 0, 0); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(0, 0, 0, 0); // 11

	// left
	v[i] = XMFLOAT3(-s,  s,  s); n[i] = XMFLOAT3(-1.0f, 0, 0); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(1, 1, 0, 0); // 12
	v[i] = XMFLOAT3(-s, -s,  s); n[i] = XMFLOAT3(-1.0f, 0, 0); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(1, 0, 0, 0); // 13
	v[i] = XMFLOAT3(-s,  s, -s); n[i] = XMFLOAT3(-1.0f, 0, 0); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(0, 1, 0, 0); // 14
	v[i] = XMFLOAT3(-s, -s, -s); n[i] = XMFLOAT3(-1.0f, 0, 0); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(0, 0, 0, 0); // 15

	// forward
	v[i] = XMFLOAT3( s,  s, -s); n[i] = XMFLOAT3(0, 0, -1.0f); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(1, 1, 0, 0); // 16
	v[i] = XMFLOAT3(-s,  s, -s); n[i] = XMFLOAT3(0, 0, -1.0f); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(1, 0, 0, 0); // 17
	v[i] = XMFLOAT3( s, -s, -s); n[i] = XMFLOAT3(0, 0, -1.0f); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(0, 1, 0, 0); // 18
	v[i] = XMFLOAT3(-s, -s, -s); n[i] = XMFLOAT3(0, 0, -1.0f); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(0, 0, 0, 0); // 19

	// back
	v[i] = XMFLOAT3( s,  s, s); n[i] = XMFLOAT3(0, 0, 1.0f); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(1, 1, 0, 0); // 20
	v[i] = XMFLOAT3( s, -s, s); n[i] = XMFLOAT3(0, 0, 1.0f); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(0, 1, 0, 0); // 21
	v[i] = XMFLOAT3(-s,  s, s); n[i] = XMFLOAT3(0, 0, 1.0f); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(1, 0, 0, 0); // 22
	v[i] = XMFLOAT3(-s, -s, s); n[i] = XMFLOAT3(0, 0, 1.0f); g[i] = XMFLOAT3(0, -1, 1); t[i++] = XMFLOAT4(0, 0, 0, 0); // 23

	AddTriangle(si + 1,  si + 0,  si + 2, submesh);
	AddTriangle(si + 1,  si + 2,  si + 3, submesh);
	AddTriangle(si + 5,  si + 4,  si + 6, submesh);
	AddTriangle(si + 5,  si + 6,  si + 7, submesh);
	AddTriangle(si + 10, si + 8,  si + 9, submesh);
	AddTriangle(si + 10, si + 9,  si + 11, submesh);
	AddTriangle(si + 13, si + 12, si + 14, submesh);
	AddTriangle(si + 13, si + 14, si + 15, submesh);
	AddTriangle(si + 17, si + 16, si + 18, submesh);
	AddTriangle(si + 17, si + 18, si + 19, submesh);
	AddTriangle(si + 21, si + 20, si + 22, submesh);
	AddTriangle(si + 21, si + 22, si + 23, submesh);
}
void Mesh::LoadQuad(float s, unsigned int submesh, unsigned int axis) {
	HasSemantic(MESH_SEMANTIC_TEXCOORD0, true);
	HasSemantic(MESH_SEMANTIC_NORMAL, true);
	HasSemantic(MESH_SEMANTIC_TANGENT, true);
	int si = VertexCount();
	VertexCount(VertexCount() + 4);

	XMFLOAT3* v = GetVertices();
	XMFLOAT4* t = GetTexcoords(0);
	XMFLOAT3* n = GetNormals();
	XMFLOAT3* g = GetTangents();

	int i = si;
	switch (axis) {
	case 0:
		v[i] = XMFLOAT3(0, -s, -s); n[i] = XMFLOAT3(1, 0, 0); g[i] = XMFLOAT3(0, 1, 1); t[i++] = XMFLOAT4(0, 1, 0, 0);
		v[i] = XMFLOAT3(0, -s,  s); n[i] = XMFLOAT3(1, 0, 0); g[i] = XMFLOAT3(0, 1, 1); t[i++] = XMFLOAT4(0, 0, 0, 0);
		v[i] = XMFLOAT3(0,  s,  s); n[i] = XMFLOAT3(1, 0, 0); g[i] = XMFLOAT3(0, 1, 1); t[i++] = XMFLOAT4(1, 0, 0, 0);
		v[i] = XMFLOAT3(0,  s, -s); n[i] = XMFLOAT3(1, 0, 0); g[i] = XMFLOAT3(0, 1, 1); t[i++] = XMFLOAT4(1, 1, 0, 0);
		break;
	case 1:
		v[i] = XMFLOAT3(-s, 0, -s); n[i] = XMFLOAT3(0, 1, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(0, 1, 0, 0);
		v[i] = XMFLOAT3(-s, 0,  s); n[i] = XMFLOAT3(0, 1, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(0, 0, 0, 0);
		v[i] = XMFLOAT3( s, 0,  s); n[i] = XMFLOAT3(0, 1, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(1, 0, 0, 0);
		v[i] = XMFLOAT3( s, 0, -s); n[i] = XMFLOAT3(0, 1, 0); g[i] = XMFLOAT3(1, 0, 1); t[i++] = XMFLOAT4(1, 1, 0, 0);
		break;
	default:
	case 2:
		v[i] = XMFLOAT3(-s, -s, 0); n[i] = XMFLOAT3(0, 0, 1); g[i] = XMFLOAT3(1, 1, 0); t[i++] = XMFLOAT4(0, 1, 0, 0);
		v[i] = XMFLOAT3(-s,  s, 0); n[i] = XMFLOAT3(0, 0, 1); g[i] = XMFLOAT3(1, 1, 0); t[i++] = XMFLOAT4(0, 0, 0, 0);
		v[i] = XMFLOAT3( s,  s, 0); n[i] = XMFLOAT3(0, 0, 1); g[i] = XMFLOAT3(1, 1, 0); t[i++] = XMFLOAT4(1, 0, 0, 0);
		v[i] = XMFLOAT3( s, -s, 0); n[i] = XMFLOAT3(0, 0, 1); g[i] = XMFLOAT3(1, 1, 0); t[i++] = XMFLOAT4(1, 1, 0, 0);
		break;
	}
	AddTriangle(si + 0, si + 1, si + 2, submesh);
	AddTriangle(si + 0, si + 2, si + 3, submesh);
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

	XMFLOAT3 min;
	XMFLOAT3 max;
	for (unsigned int i = 0; i < mVertices.size(); i++){
		if (i == 0) {
			min = mVertices[i];
			max = mVertices[i];
		} else {
			min.x = fmin(mVertices[i].x, min.x);
			min.y = fmin(mVertices[i].y, min.y);
			min.z = fmin(mVertices[i].z, min.z);
			max.x = fmax(mVertices[i].x, max.x);
			max.y = fmax(mVertices[i].y, max.y);
			max.z = fmax(mVertices[i].z, max.z);
		}
	}
	mBounds = BoundingBox(
		XMFLOAT3((min.x + max.x) * .5f, (min.y + max.y) * .5f, (min.z + max.z) * .5f),
		XMFLOAT3((max.x - min.x) * .5f, (max.y - min.y) * .5f, (max.z - min.z) * .5f));

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

	unsigned int ic = 0;
	for (unsigned int s = 0; s < mSubmeshes.size(); s++)
		ic += IndexCount(s);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ic * isize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mIndexBuffer)));

	ComPtr<ID3D12Resource> iintermediate;
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ic * isize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&iintermediate)));

	void* idata;
	CD3DX12_RANGE ireadRange(0, 0);
	iintermediate->Map(0, &ireadRange, &idata);
	unsigned int c = 0;
	for (unsigned int s = 0; s < mSubmeshes.size(); s++) {
		Submesh &sm = mSubmeshes[s];

		sm.mIndexCount = IndexCount(s);
		sm.mStartIndex = c;

		if (Use32BitIndices())
			memcpy((uint32_t*)idata + c, sm.mIndices32.data(), sm.mIndexCount * isize);
		else
			memcpy((uint16_t*)idata + c, sm.mIndices16.data(), sm.mIndexCount * isize);

		c += sm.mIndexCount;
	}

	iintermediate->Unmap(0, &ireadRange);

	commandList->D3DCommandList()->CopyResource(mIndexBuffer.Get(), iintermediate.Get());
	commandList->TransitionResource(mIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.SizeInBytes = c * (UINT)isize;
	mIndexBufferView.Format = Use32BitIndices() ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	mIndexBuffer->SetName(L"Index Buffer");
	#pragma endregion

	commandQueue->WaitForFenceValue(commandQueue->Execute(commandList));

	mVertexBuffer->SetName(L"Vertex Buffer");

	mDataUploaded = true;
}

void Mesh::ReleaseGpu() {
	mVertexBuffer.Reset();
	mIndexBuffer.Reset();
	mDataUploaded = false;
}