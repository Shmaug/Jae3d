#include "MeshAsset.hpp"
#include "AssetFile.hpp"
#include "MemoryStream.hpp"

using namespace std;
using namespace DirectX;

MeshAsset::MeshAsset(string name) : Asset(name) {}
MeshAsset::MeshAsset(string name, MemoryStream &ms) : Asset(name) {
	uint32_t bonec = ms.Read<uint32_t>();
	bones.reserve(bonec);
	for (uint32_t i = 0; i < bonec; i++) {
		string name = ms.Read<string>();
		float data[16];
		ms.Read(reinterpret_cast<char*>(data), sizeof(data));
		bones.push_back(Bone(name, XMFLOAT4X4(data)));
	}

	m_Flags = (MESHFLAGS)ms.Read<uint32_t>();
	m_TexMask = ms.Read<uint8_t>();
	VertexCount(ms.Read<uint32_t>());

	for (unsigned int i = 0; i < VertexCount(); i++) {
		m_Vertices[i] = ms.Read<XMFLOAT3>();
		if (HasNormals()) m_Normals[i] = ms.Read<XMFLOAT3>();
		if (HasTangents()) m_Tangents[i] = ms.Read<XMFLOAT3>();
		if (HasBitangents()) m_Bitangents[i] = ms.Read<XMFLOAT3>();
		if (HasColors()) m_Colors[i] = ms.Read<XMFLOAT4>();
		if (HasBones()) {
			m_BlendIndices[i] = ms.Read<XMINT4>();
			m_BlendWeights[i] = ms.Read<XMFLOAT4>();
		}
		for (int j = 0; j < 8; j++)
			if (HasTexcoords(j))
				m_Texcoords[j][i] = ms.Read<XMFLOAT4>();
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
}
MeshAsset::~MeshAsset() {
	Clear();
}
uint64_t MeshAsset::TypeId() { return (uint64_t)AssetFile::TYPEID_MESH; }

void MeshAsset::WriteData(MemoryStream &ms) {
	Asset::WriteData(ms);

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

	ms.Write((uint32_t)m_Flags);
	ms.Write(m_TexMask);
	ms.Write((uint32_t)VertexCount());

	for (unsigned int i = 0; i < VertexCount(); i++) {
		ms.Write(m_Vertices[i]);
		if (HasNormals()) ms.Write(m_Normals[i]);
		if (HasTangents()) ms.Write(m_Tangents[i]);
		if (HasBitangents()) ms.Write(m_Bitangents[i]);
		if (HasColors()) ms.Write(m_Colors[i]);
		if (HasBones()) {
			ms.Write(m_BlendIndices[i]);
			ms.Write(m_BlendWeights[i]);
		}
		for (int j = 0; j < 8; j++)
			if (HasTexcoords(j))
				ms.Write(m_Texcoords[j][i]);
	}

	ms.Write((uint32_t)IndexCount());
	ms.Write((uint8_t)m_32BitIndices);
	if (m_32BitIndices)
		for (unsigned int i = 0; i < IndexCount(); i++)
			ms.Write(m_Indices32[i]);
	else
		for (unsigned int i = 0; i < IndexCount(); i++)
			ms.Write(m_Indices16[i]);
}

void MeshAsset::Clear() {
	m_Vertices.swap(vector<XMFLOAT3>());
	m_Normals.swap(vector<XMFLOAT3>());
	m_Tangents.swap(vector<XMFLOAT3>());
	m_Bitangents.swap(vector<XMFLOAT3>());
	m_Colors.swap(vector<XMFLOAT4>());
	m_BlendIndices.swap(vector<XMINT4>());
	m_BlendWeights.swap(vector<XMFLOAT4>());
	bones.swap(vector<Bone>());
	for (int i = 0; i < 8; i++)
		m_Texcoords[i].swap(vector<XMFLOAT4>());
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
	if (HasNormals()) m_Normals.resize(size);
	if (HasTangents()) m_Tangents.resize(size);
	if (HasBitangents()) m_Bitangents.resize(size);
	if (HasColors()) m_Colors.resize(size);
	if (HasBones()) {
		m_BlendIndices.resize(size);
		m_BlendWeights.resize(size);
	}
	for (int i = 0; i < 8; i++)
		if (HasTexcoords(i))
			m_Texcoords[i].resize(size);

	if (size < m_Vertices.size() && shrink) {
		m_Vertices.shrink_to_fit();
		if (HasNormals()) m_Normals.shrink_to_fit();
		if (HasTangents()) m_Tangents.shrink_to_fit();
		if (HasBitangents()) m_Bitangents.shrink_to_fit();
		if (HasColors()) m_Colors.shrink_to_fit();
		if (HasBones()) {
			m_BlendIndices.shrink_to_fit();
			m_BlendWeights.shrink_to_fit();
		}
		for (int i = 0; i < 8; i++)
			if (HasTexcoords(i))
				m_Texcoords[i].shrink_to_fit();
	}
}
unsigned int MeshAsset::AddVertex(XMFLOAT3 &v){
	m_Vertices.push_back(v);
	if (HasNormals()) m_Normals.push_back(DirectX::XMFLOAT3());
	if (HasTangents()) m_Tangents.push_back(DirectX::XMFLOAT3());
	if (HasBitangents()) m_Bitangents.push_back(DirectX::XMFLOAT3());
	if (HasColors()) m_Colors.push_back(DirectX::XMFLOAT4());
	if (HasBones()) { m_BlendIndices.push_back(DirectX::XMINT4()); m_BlendWeights.push_back(DirectX::XMFLOAT4()); }
	for (int i = 0; i < 8; i++)
		if (HasTexcoords(i)) { m_Texcoords[i].push_back(DirectX::XMFLOAT4()); }
	return (unsigned int)m_Vertices.size() - 1;
}