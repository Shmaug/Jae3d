#include "MeshAsset.hpp"
#include "AssetFile.hpp"
#include "MemoryStream.hpp"

using namespace std;
using namespace DirectX;

MeshAsset::MeshAsset(jstring name) : Asset(name) {}
MeshAsset::MeshAsset(jstring name, MemoryStream &ms) : Asset(name) {
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
		jstring name = ms.Read<jstring>();
		float data[16];
		ms.Read(reinterpret_cast<char*>(data), sizeof(data));
		bones.push_back(Bone(name, XMFLOAT4X4(data)));
	}
}
MeshAsset::~MeshAsset() {
	Clear();
}
uint64_t MeshAsset::TypeId() { return (uint64_t)AssetFile::TYPEID_MESH; }

void MeshAsset::WriteData(MemoryStream &ms) {
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

void MeshAsset::Clear() {
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

void MeshAsset::Use32BitIndices(bool v) {
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

void MeshAsset::VertexCount(unsigned int size, bool shrink) {
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

	/*
	if (size < mVertices.size() && shrink) {
		mVertices.shrink_to_fit();
		if (HasSemantic(SEMANTIC_NORMAL)) mNormals.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TANGENT)) mTangents.shrink_to_fit();
		if (HasSemantic(SEMANTIC_BINORMAL)) mBinormals.shrink_to_fit();
		if (HasSemantic(SEMANTIC_COLOR0)) mColor0.shrink_to_fit();
		if (HasSemantic(SEMANTIC_COLOR1)) mColor1.shrink_to_fit();
		if (HasSemantic(SEMANTIC_BLENDINDICES)) mBlendIndices.shrink_to_fit();
		if (HasSemantic(SEMANTIC_BLENDWEIGHT)) mBlendWeights.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TEXCOORD0)) mTexcoord0.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TEXCOORD1)) mTexcoord1.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TEXCOORD2)) mTexcoord2.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TEXCOORD3)) mTexcoord3.shrink_to_fit();
	}
	*/
}
unsigned int MeshAsset::AddVertex(XMFLOAT3 &v){
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