#include "MeshAsset.hpp"

using namespace std;
using namespace DirectX;

MeshAsset::MeshAsset(string name) : Asset(name) {}
MeshAsset::~MeshAsset() {
	Clear();
}

void MeshAsset::WriteHeader(ofstream &stream) {
	Asset::WriteHeader(stream);

}
void MeshAsset::WriteData(ofstream &stream) {
	Asset::WriteData(stream);

}

void MeshAsset::Clear() {
	m_Vertices.clear();
	m_Normals.clear();
	m_Tangents.clear();
	m_Bitangents.clear();
	m_Colors.clear();
	m_BlendIndices.clear();
	m_BlendWeights.clear();
	bones.clear();
	for (int i = 0; i < 8; i++)
		m_Texcoords[i].clear();
	m_Indices.clear();
}