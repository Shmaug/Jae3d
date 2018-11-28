#pragma once

#include <vector>
#include <string>
#include <DirectXMath.h>

#include "Asset.hpp"

class MeshAsset : public Asset {
public:
	enum SEMANTIC {
		SEMANTIC_POSITION = 0, // always have position
		SEMANTIC_NORMAL = 1,
		SEMANTIC_TANGENT = 2,
		SEMANTIC_BINORMAL = 4,
		SEMANTIC_COLOR0 = 8,
		SEMANTIC_COLOR1 = 16,
		SEMANTIC_BLENDINDICES = 32,
		SEMANTIC_BLENDWEIGHT = 64,
		SEMANTIC_TEXCOORD0 = 128,
		SEMANTIC_TEXCOORD1 = 256,
		SEMANTIC_TEXCOORD2 = 512,
		SEMANTIC_TEXCOORD3 = 1024,
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
	bool Use32BitIndices() const { return m_32BitIndices; }
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
	unsigned int IndexCount() const { return(unsigned int)(m_32BitIndices ? m_Indices32.size() : m_Indices16.size()); }
	void VertexCount(unsigned int size, bool shrink = true);
	
	unsigned int AddVertex(DirectX::XMFLOAT3 &v);

	void HasSemantic(SEMANTIC s, bool v) {
		if (s == SEMANTIC_POSITION || HasSemantic(s) == v) return;
		if (v) {
			m_Semantics = (SEMANTIC)(m_Semantics | s);
			switch (s) {
			case SEMANTIC_NORMAL:
				m_Normals.resize(VertexCount());
				break;
			case SEMANTIC_TANGENT:
				m_Tangents.resize(VertexCount());
				break;
			case SEMANTIC_BINORMAL:
				m_Binormals.resize(VertexCount());
				break;
			case SEMANTIC_COLOR0:
				m_Color0.resize(VertexCount());
				break;
			case SEMANTIC_COLOR1:
				m_Color1.swap(std::vector<DirectX::XMFLOAT4>(VertexCount()));
				break;
			case SEMANTIC_BLENDINDICES:
				m_BlendIndices.resize(VertexCount());
				break;
			case SEMANTIC_BLENDWEIGHT:
				m_BlendWeights.resize(VertexCount());
				break;
			case SEMANTIC_TEXCOORD0:
				m_Texcoord0.resize(VertexCount());
				break;
			case SEMANTIC_TEXCOORD1:
				m_Texcoord1.resize(VertexCount());
				break;
			case SEMANTIC_TEXCOORD2:
				m_Texcoord2.resize(VertexCount());
				break;
			case SEMANTIC_TEXCOORD3:
				m_Texcoord3.resize(VertexCount());
				break;
			}
		} else {
			m_Semantics = (SEMANTIC)(m_Semantics & ~s);
			switch (s) {
			case SEMANTIC_NORMAL:
				m_Normals.swap(std::vector<DirectX::XMFLOAT3>());
				break;
			case SEMANTIC_TANGENT:
				m_Tangents.swap(std::vector<DirectX::XMFLOAT3>(VertexCount()));
				break;
			case SEMANTIC_BINORMAL:
				m_Binormals.swap(std::vector<DirectX::XMFLOAT3>(VertexCount()));
				break;
			case SEMANTIC_COLOR0:
				m_Color0.swap(std::vector<DirectX::XMFLOAT4>(VertexCount()));
				break;
			case SEMANTIC_COLOR1:
				m_Color1.swap(std::vector<DirectX::XMFLOAT4>(VertexCount()));
				break;
			case SEMANTIC_BLENDINDICES:
				m_BlendIndices.swap(std::vector<DirectX::XMUINT4>(VertexCount()));
				break;
			case SEMANTIC_BLENDWEIGHT:
				m_BlendWeights.swap(std::vector<DirectX::XMFLOAT4>(VertexCount()));
				break;
			case SEMANTIC_TEXCOORD0:
				m_Texcoord0.swap(std::vector<DirectX::XMFLOAT4>(VertexCount()));
				break;
			case SEMANTIC_TEXCOORD1:
				m_Texcoord1.swap(std::vector<DirectX::XMFLOAT4>(VertexCount()));
				break;
			case SEMANTIC_TEXCOORD2:
				m_Texcoord2.swap(std::vector<DirectX::XMFLOAT4>(VertexCount()));
				break;
			case SEMANTIC_TEXCOORD3:
				m_Texcoord3.swap(std::vector<DirectX::XMFLOAT4>(VertexCount()));
				break;
			}
		}
	}
	bool HasSemantic(SEMANTIC s) const { if (s == SEMANTIC_POSITION) return true; return m_Semantics & s; }
	SEMANTIC Semantics() const { return m_Semantics; }

	template<typename T>
	T* GetSemantic(SEMANTIC s) {
		switch (s) {
		case SEMANTIC_POSITION:
			static_assert(std::is_same(T, DirectX::XMFLOAT3), "T must be XMFLOAT3");
			return m_Vertices.data();
			break;
		case SEMANTIC_NORMAL:
			static_assert(std::is_same(T, DirectX::XMFLOAT3), "T must be XMFLOAT3");
			return m_Normals.data();
			break;
		case SEMANTIC_TANGENT:
			static_assert(std::is_same(T, DirectX::XMFLOAT3), "T must be XMFLOAT3");
			return m_Tangents.data();
			break;
		case SEMANTIC_BINORMAL:
			static_assert(std::is_same(T, DirectX::XMFLOAT3), "T must be XMFLOAT3");
			return m_Binormals.data();
			break;
		case SEMANTIC_COLOR0:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return m_Color0.data();
			break;
		case SEMANTIC_COLOR1:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return m_Color1.data();
			break;
		case SEMANTIC_BLENDINDICES:
			static_assert(std::is_same(T, DirectX::XMUINT4), "T must be XMUINT4");
			return m_BlendIndices.data();
			break;
		case SEMANTIC_BLENDWEIGHT:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return m_BlendWeights.data();
			break;
		case SEMANTIC_TEXCOORD0:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return m_Texcoord0.data();
			break;
		case SEMANTIC_TEXCOORD1:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return m_Texcoord1.data();
			break;
		case SEMANTIC_TEXCOORD2:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return m_Texcoord2.data();
			break;
		case SEMANTIC_TEXCOORD3:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return m_Texcoord3.data();
			break;
		}
	}

	DirectX::XMFLOAT3* GetVertices() { return m_Vertices.data(); }
	DirectX::XMFLOAT3* GetNormals() { return m_Normals.data(); }
	DirectX::XMFLOAT3* GetTangents() { return m_Tangents.data(); }
	DirectX::XMFLOAT3* GetBinormals() { return m_Binormals.data(); }
	DirectX::XMFLOAT4* GetTexcoords(const int channel) {
		switch (channel) {
		default:
		case 0: return m_Texcoord0.data();
		case 1: return m_Texcoord1.data();
		case 2: return m_Texcoord2.data();
		case 3: return m_Texcoord3.data();
		}
	}
	DirectX::XMFLOAT4* GetColors(const int channel) {
		switch (channel) {
		default:
		case 0: return m_Color0.data();
		case 1: return m_Color1.data();
		}
	}
	DirectX::XMUINT4*  GetBlendIndices() { return m_BlendIndices.data(); }
	DirectX::XMFLOAT4* GetBlendWeights() { return m_BlendWeights.data(); }

	uint16_t* GetIndices16() { return m_Indices16.data(); }
	uint32_t* GetIndices32() { return m_Indices32.data(); }
#pragma endregion

private:
	bool m_32BitIndices = false;

	SEMANTIC m_Semantics = SEMANTIC_POSITION;

	std::vector<Bone> bones;
	std::vector<DirectX::XMFLOAT3> m_Vertices;
	std::vector<DirectX::XMFLOAT3> m_Normals;
	std::vector<DirectX::XMFLOAT3> m_Tangents;
	std::vector<DirectX::XMFLOAT3> m_Binormals;
	std::vector<DirectX::XMFLOAT4> m_Texcoord0;
	std::vector<DirectX::XMFLOAT4> m_Texcoord1;
	std::vector<DirectX::XMFLOAT4> m_Texcoord2;
	std::vector<DirectX::XMFLOAT4> m_Texcoord3;
	std::vector<DirectX::XMFLOAT4> m_Color0;
	std::vector<DirectX::XMFLOAT4> m_Color1;
	std::vector<DirectX::XMUINT4>  m_BlendIndices;
	std::vector<DirectX::XMFLOAT4> m_BlendWeights;
	std::vector<uint16_t> m_Indices16;
	std::vector<uint32_t> m_Indices32;
};

