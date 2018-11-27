#pragma once

#include <vector>
#include <string>
#include <DirectXMath.h>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>

#include "Asset.hpp"

class MeshAsset : public Asset {
public:
	enum MESHFLAGS {
		MESHFLAG_NONE = 0,
		MESHFLAG_HASNORMALS = 1,
		MESHFLAG_HASTANGENTS = 2,
		MESHFLAG_HASBITANGENTS = 4,
		MESHFLAG_HASCOLORS = 8,
		MESHFLAG_HASBONES = 16,
	};

	struct Bone {
		std::string m_Name;
		DirectX::XMFLOAT4X4 m_BoneToMesh;
		Bone(std::string name, DirectX::XMFLOAT4X4 m) : m_Name(name), m_BoneToMesh(m) {}
	};

	MeshAsset(std::string name);
	MeshAsset(std::string name, MemoryStream &ms);
	~MeshAsset();

#pragma region IO
	void WriteData(MemoryStream &ms);
	uint64_t TypeId();
#pragma endregion

#pragma region Getters/Setters
	void Clear();

	void Use32BitIndices(bool v);
	void AddTriangle(uint16_t i0, uint16_t i1, uint16_t i2) {
		if (m_32BitIndices) {
			m_Indices32.push_back((uint32_t)i0);
			m_Indices32.push_back((uint32_t)i1);
			m_Indices32.push_back((uint32_t)i2);
		} else {
			m_Indices16.push_back(i0);
			m_Indices16.push_back(i1);
			m_Indices16.push_back(i2);
		}
	}
	void AddTriangle(uint32_t i0, uint32_t i1, uint32_t i2) {
		if (m_32BitIndices) {
			m_Indices32.push_back(i0);
			m_Indices32.push_back(i1);
			m_Indices32.push_back(i2);
		} else {
			m_Indices16.push_back((uint16_t)i0);
			m_Indices16.push_back((uint16_t)i1);
			m_Indices16.push_back((uint16_t)i2);
		}
	}

	unsigned int VertexCount() const { return (unsigned int)m_Vertices.size(); }
	unsigned int IndexCount() const { if (m_32BitIndices) return (unsigned int)m_Indices32.size(); else return (unsigned int)m_Indices16.size(); }
	void VertexCount(unsigned int size, bool shrink = true);
	
	unsigned int AddVertex(DirectX::XMFLOAT3 &v);

	void HasNormals(bool v) {
		if (HasNormals() == v) return;
		if (v) {
			m_Flags = (MESHFLAGS)(m_Flags | MESHFLAG_HASNORMALS);
			m_Normals.resize(VertexCount());
		} else {
			m_Flags = (MESHFLAGS)(m_Flags & ~MESHFLAG_HASNORMALS);
			m_Normals.swap(std::vector<DirectX::XMFLOAT3>());
		}
	}
	void HasTangents(bool v) {
		if (HasTangents() == v) return;
		if (v) {
			m_Flags = (MESHFLAGS)(m_Flags | MESHFLAG_HASTANGENTS);
			m_Tangents.resize(VertexCount());
		} else {
			m_Flags = (MESHFLAGS)(m_Flags & ~MESHFLAG_HASTANGENTS);
			m_Tangents.swap(std::vector<DirectX::XMFLOAT3>());
		}
	}
	void HasBitangents(bool v) {
		if (HasBitangents() == v) return;
		if (v) {
			m_Flags = (MESHFLAGS)(m_Flags | MESHFLAG_HASBITANGENTS);
			m_Bitangents.resize(VertexCount());
		} else {
			m_Flags = (MESHFLAGS)(m_Flags & ~MESHFLAG_HASBITANGENTS);
			m_Bitangents.swap(std::vector<DirectX::XMFLOAT3>());
		}
	}
	void HasTexcoords(int i, bool v) {
		if (HasTexcoords(i) == v) return;
		if (v) {
			m_TexMask |= 1 << i;
			m_Texcoords[i].resize(VertexCount());
		} else {
			m_TexMask &= ~(1 << i);
			m_Texcoords[i].swap(std::vector<DirectX::XMFLOAT4>());
		}
	}
	void HasColors(bool v) {
		if (HasColors() == v) return;
		if (v) {
			m_Flags = (MESHFLAGS)(m_Flags | MESHFLAG_HASCOLORS);
			m_Colors.resize(VertexCount());
		} else {
			m_Flags = (MESHFLAGS)(m_Flags & ~MESHFLAG_HASCOLORS);
			m_Colors.swap(std::vector<DirectX::XMFLOAT4>());
		}
	}
	void HasBones(bool v) {
		if (HasBones() == v) return;
		if (v) {
			m_Flags = (MESHFLAGS)(m_Flags | MESHFLAG_HASBONES);
			m_BlendIndices.resize(VertexCount());
			m_BlendWeights.resize(VertexCount());
		} else {
			m_Flags = (MESHFLAGS)(m_Flags & ~MESHFLAG_HASBONES);
			m_BlendIndices.swap(std::vector<DirectX::XMINT4>());
			m_BlendWeights.swap(std::vector<DirectX::XMFLOAT4>());
		}
	}

	bool HasNormals() const { return m_Flags & MESHFLAG_HASNORMALS; }
	bool HasTangents() const { return m_Flags & MESHFLAG_HASTANGENTS; }
	bool HasBitangents() const { return m_Flags & MESHFLAG_HASBITANGENTS; }
	bool HasTexcoords(const int channel) const { return m_TexMask & (1 << channel); }
	bool HasColors() const { return m_Flags & MESHFLAG_HASCOLORS; }
	bool HasBones() const { return m_Flags & MESHFLAG_HASBONES; }

	DirectX::XMFLOAT3* GetVertices() { return m_Vertices.data(); }
	DirectX::XMFLOAT3* GetNormals() { return m_Normals.data(); }
	DirectX::XMFLOAT3* GetTangents() { return m_Tangents.data(); }
	DirectX::XMFLOAT3* GetBitangents() { return m_Bitangents.data(); }
	DirectX::XMFLOAT4* GetTexcoords(const int channel) { return m_Texcoords[channel].data(); }
	DirectX::XMFLOAT4* GetColors() { return m_Colors.data(); }
	DirectX::XMINT4*   GetBlendIndices() { return m_BlendIndices.data(); }
	DirectX::XMFLOAT4* GetBlendWeights() { return m_BlendWeights.data(); }
#pragma endregion

private:
	bool m_32BitIndices = false;

	MESHFLAGS m_Flags;
	uint8_t m_TexMask;

	std::vector<Bone> bones;
	std::vector<DirectX::XMFLOAT3> m_Vertices;
	std::vector<DirectX::XMFLOAT3> m_Normals;
	std::vector<DirectX::XMFLOAT3> m_Tangents;
	std::vector<DirectX::XMFLOAT3> m_Bitangents;
	std::vector<DirectX::XMFLOAT4> m_Texcoords[8];
	std::vector<DirectX::XMFLOAT4> m_Colors;
	std::vector<DirectX::XMINT4>   m_BlendIndices;
	std::vector<DirectX::XMFLOAT4> m_BlendWeights;
	std::vector<uint16_t> m_Indices16;
	std::vector<uint32_t> m_Indices32;
};

