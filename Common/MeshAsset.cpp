#include "MeshAsset.hpp"
#include "AssetFile.hpp"
#include "MemoryStream.hpp"

using namespace std;
using namespace DirectX;

MeshAsset::MeshAsset(string name) : Asset(name) {}
MeshAsset::MeshAsset(string name, MemoryStream &ms) : Asset(name) {
	m_Semantics = (SEMANTIC)ms.Read<uint32_t>();
	VertexCount(ms.Read<uint32_t>());

	for (unsigned int i = 0; i < VertexCount(); i++) {
		m_Vertices[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(SEMANTIC_NORMAL)) m_Normals[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(SEMANTIC_TANGENT)) m_Tangents[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(SEMANTIC_BINORMAL)) m_Binormals[i] = ms.Read<XMFLOAT3>();
		if (HasSemantic(SEMANTIC_COLOR0)) m_Color0[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_COLOR1)) m_Color1[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_BLENDINDICES)) m_BlendIndices[i] = ms.Read<XMUINT4>();
		if (HasSemantic(SEMANTIC_BLENDWEIGHT)) m_BlendWeights[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_TEXCOORD0)) m_Texcoord0[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_TEXCOORD1)) m_Texcoord1[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_TEXCOORD2)) m_Texcoord2[i] = ms.Read<XMFLOAT4>();
		if (HasSemantic(SEMANTIC_TEXCOORD3)) m_Texcoord3[i] = ms.Read<XMFLOAT4>();
	}

	uint32_t indexCount = ms.Read<uint32_t>();
	m_32BitIndices = ms.Read<uint8_t>();
	if (m_32BitIndices) {
		m_Indices32.reserve(indexCount);
		for (uint32_t i = 0; i < indexCount; i++)
			m_Indices32.push_back(ms.Read<uint32_t>());
	} else {
		m_Indices16.reserve(indexCount);
		for (uint32_t i = 0; i < indexCount; i++)
			m_Indices16.push_back(ms.Read<uint16_t>());
	}

	uint32_t bonec = ms.Read<uint32_t>();
	bones.reserve(bonec);
	for (uint32_t i = 0; i < bonec; i++) {
		string name = ms.Read<string>();
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

	ms.Write((uint32_t)m_Semantics);
	ms.Write((uint32_t)VertexCount());

	for (unsigned int i = 0; i < VertexCount(); i++) {
		ms.Write(m_Vertices[i]);
		if (HasSemantic(SEMANTIC_NORMAL)) ms.Write(m_Normals[i]);
		if (HasSemantic(SEMANTIC_TANGENT)) ms.Write(m_Tangents[i]);
		if (HasSemantic(SEMANTIC_BINORMAL)) ms.Write(m_Binormals[i]);
		if (HasSemantic(SEMANTIC_COLOR0)) ms.Write(m_Color0[i]);
		if (HasSemantic(SEMANTIC_COLOR1)) ms.Write(m_Color1[i]);
		if (HasSemantic(SEMANTIC_BLENDINDICES)) ms.Write(m_BlendIndices[i]);
		if (HasSemantic(SEMANTIC_BLENDWEIGHT)) ms.Write(m_BlendWeights[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD0)) ms.Write(m_Texcoord0[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD1)) ms.Write(m_Texcoord1[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD2)) ms.Write(m_Texcoord2[i]);
		if (HasSemantic(SEMANTIC_TEXCOORD3)) ms.Write(m_Texcoord3[i]);
	}

	ms.Write((uint32_t)IndexCount());
	ms.Write((uint8_t)m_32BitIndices);
	if (m_32BitIndices)
		for (unsigned int i = 0; i < IndexCount(); i++)
			ms.Write(m_Indices32[i]);
	else
		for (unsigned int i = 0; i < IndexCount(); i++)
			ms.Write(m_Indices16[i]);

	ms.Write((uint32_t)bones.size());
	for (int i = 0; i < bones.size(); i++) {
		ms.Write(bones[i].m_Name);
		float data[16] = {
			bones[i].m_BoneToMesh._11, bones[i].m_BoneToMesh._12, bones[i].m_BoneToMesh._13, bones[i].m_BoneToMesh._14,
			bones[i].m_BoneToMesh._21, bones[i].m_BoneToMesh._22, bones[i].m_BoneToMesh._23, bones[i].m_BoneToMesh._24,
			bones[i].m_BoneToMesh._31, bones[i].m_BoneToMesh._32, bones[i].m_BoneToMesh._33, bones[i].m_BoneToMesh._34,
			bones[i].m_BoneToMesh._41, bones[i].m_BoneToMesh._42, bones[i].m_BoneToMesh._43, bones[i].m_BoneToMesh._44
		};
		ms.Write(reinterpret_cast<const char*>(data), sizeof(data));
	}
}

void MeshAsset::Clear() {
	bones.swap(vector<Bone>());
	m_Vertices.swap(vector<XMFLOAT3>());
	m_Normals.swap(vector<XMFLOAT3>());
	m_Tangents.swap(vector<XMFLOAT3>());
	m_Binormals.swap(vector<XMFLOAT3>());
	m_Color0.swap(vector<XMFLOAT4>());
	m_Color1.swap(vector<XMFLOAT4>());
	m_BlendIndices.swap(vector<XMUINT4>());
	m_BlendWeights.swap(vector<XMFLOAT4>());
	m_Texcoord0.swap(vector<XMFLOAT4>());
	m_Texcoord1.swap(vector<XMFLOAT4>());
	m_Texcoord2.swap(vector<XMFLOAT4>());
	m_Texcoord3.swap(vector<XMFLOAT4>());

	m_Indices16.swap(vector<uint16_t>());
	m_Indices32.swap(vector<uint32_t>());
}

void MeshAsset::Use32BitIndices(bool v) {
	if (m_32BitIndices == v) return;
	m_32BitIndices = v;
	if (v) {
		// switch to 32 bit indices
		m_Indices32.reserve(m_Indices16.size());
		for (int i = 0; i < m_Indices16.size(); i++)
			m_Indices32.push_back((uint32_t)m_Indices16[i]);
		m_Indices16.swap(vector<uint16_t>());
	} else {
		// switch to 16 bit indices
		m_Indices16.reserve(m_Indices32.size());
		for (int i = 0; i < m_Indices32.size(); i++)
			m_Indices16.push_back((uint16_t)m_Indices32[i]);
		m_Indices32.swap(vector<uint32_t>());
	}
}

void MeshAsset::VertexCount(unsigned int size, bool shrink) {
	if (size == m_Vertices.size()) return;

	m_Vertices.resize(size);
	if (HasSemantic(SEMANTIC_NORMAL)) m_Normals.resize(size);
	if (HasSemantic(SEMANTIC_TANGENT)) m_Tangents.resize(size);
	if (HasSemantic(SEMANTIC_BINORMAL)) m_Binormals.resize(size);
	if (HasSemantic(SEMANTIC_COLOR0)) m_Color0.resize(size);
	if (HasSemantic(SEMANTIC_COLOR1)) m_Color1.resize(size);
	if (HasSemantic(SEMANTIC_BLENDINDICES)) m_BlendIndices.resize(size);
	if (HasSemantic(SEMANTIC_BLENDWEIGHT)) m_BlendWeights.resize(size);
	if (HasSemantic(SEMANTIC_TEXCOORD0)) m_Texcoord0.resize(size);
	if (HasSemantic(SEMANTIC_TEXCOORD1)) m_Texcoord1.resize(size);
	if (HasSemantic(SEMANTIC_TEXCOORD2)) m_Texcoord2.resize(size);
	if (HasSemantic(SEMANTIC_TEXCOORD3)) m_Texcoord3.resize(size);

	if (size < m_Vertices.size() && shrink) {
		m_Vertices.shrink_to_fit();
		if (HasSemantic(SEMANTIC_NORMAL)) m_Normals.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TANGENT)) m_Tangents.shrink_to_fit();
		if (HasSemantic(SEMANTIC_BINORMAL)) m_Binormals.shrink_to_fit();
		if (HasSemantic(SEMANTIC_COLOR0)) m_Color0.shrink_to_fit();
		if (HasSemantic(SEMANTIC_COLOR1)) m_Color1.shrink_to_fit();
		if (HasSemantic(SEMANTIC_BLENDINDICES)) m_BlendIndices.shrink_to_fit();
		if (HasSemantic(SEMANTIC_BLENDWEIGHT)) m_BlendWeights.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TEXCOORD0)) m_Texcoord0.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TEXCOORD1)) m_Texcoord1.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TEXCOORD2)) m_Texcoord2.shrink_to_fit();
		if (HasSemantic(SEMANTIC_TEXCOORD3)) m_Texcoord3.shrink_to_fit();
	}
}
unsigned int MeshAsset::AddVertex(XMFLOAT3 &v){
	m_Vertices.push_back(v);
	if (HasSemantic(SEMANTIC_NORMAL)) m_Normals.push_back(DirectX::XMFLOAT3());
	if (HasSemantic(SEMANTIC_TANGENT)) m_Tangents.push_back(DirectX::XMFLOAT3());
	if (HasSemantic(SEMANTIC_BINORMAL)) m_Binormals.push_back(DirectX::XMFLOAT3());
	if (HasSemantic(SEMANTIC_COLOR0)) m_Color0.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_COLOR1)) m_Color1.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_BLENDWEIGHT)) m_BlendWeights.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_BLENDINDICES)) m_BlendIndices.push_back(DirectX::XMUINT4());
	if (HasSemantic(SEMANTIC_TEXCOORD0)) m_Texcoord0.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_TEXCOORD1)) m_Texcoord1.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_TEXCOORD2)) m_Texcoord2.push_back(DirectX::XMFLOAT4());
	if (HasSemantic(SEMANTIC_TEXCOORD3)) m_Texcoord3.push_back(DirectX::XMFLOAT4());
	return (unsigned int)m_Vertices.size() - 1;
}