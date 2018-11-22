#pragma once

#include <vector>
#include <string>
#include <DirectXMath.h>

#include "Asset.hpp"

class MeshAsset : public Asset {
public:
	struct Bone {
		std::string m_Name;
		DirectX::XMFLOAT4X4 m_BoneToMesh;
	};

	MeshAsset(std::string name);
	~MeshAsset();

	uint64_t HeaderSize();
	uint64_t DataSize();
	void WriteHeader(std::ofstream &stream);
	void WriteData(std::ofstream &stream);

	void Clear();

	void AddTriangle(int i0, int i1, int i2) {
		m_Indices.push_back(i0);
		m_Indices.push_back(i1);
		m_Indices.push_back(i2);
	}
	
	std::vector<Bone> bones;

	std::vector<DirectX::XMFLOAT3> m_Vertices;
	std::vector<DirectX::XMFLOAT3> m_Normals;
	std::vector<DirectX::XMFLOAT3> m_Tangents;
	std::vector<DirectX::XMFLOAT3> m_Bitangents;
	std::vector<DirectX::XMFLOAT4> m_Colors;
	std::vector<DirectX::XMINT4> m_BlendIndices;
	std::vector<DirectX::XMFLOAT4> m_BlendWeights;
	std::vector<DirectX::XMFLOAT3> m_Texcoords[8];

private:
	std::vector<int> m_Indices;
};

