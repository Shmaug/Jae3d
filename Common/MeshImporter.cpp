#include "MeshImporter.hpp"

#include <stack>
#include <vector>

#define ASSIMP_OBJ

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "AssetImporter.hpp"
#include "MeshAsset.hpp"
#include "IOUtil.hpp"

#include <d3d12.h>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

XMFLOAT3 ai2dx(aiVector3D v) {
	return { v.x, v.y, v.z };
}
XMFLOAT4 ai2dx(aiColor4D c) {
	return { c.r, c.g, c.b, c.a };
}
XMFLOAT4X4 ai2dx(aiMatrix4x4 m) {
	return XMFLOAT4X4(m.a1, m.a2, m.a3, m.a4, m.b1, m.b2, m.b3, m.b4, m.c1, m.c2, m.c3, m.c4, m.d1, m.d2, m.d3, m.d4);
}
void XMSET(XMFLOAT4 &v, int i, float val) {
	switch (i) {
	case 0:
		v.x = val;
		break;
	case 1:
		v.y = val;
		break;
	case 2:
		v.z = val;
		break;
	case 3:
		v.w = val;
		break;
	}
}
float XMGET(XMFLOAT4 v, int i) {
	switch (i) {
	case 0:
		return v.x;
	case 1:
		return v.y;
	case 2:
		return v.z;
	case 3:
		return v.w;
	default:
		return 0;
	}
}
void XMSET(XMINT4 &v, int i, int val) {
	switch (i) {
	case 0:
		v.x = val;
		break;
	case 1:
		v.y = val;
		break;
	case 2:
		v.z = val;
		break;
	case 3:
		v.w = val;
		break;
	}
}

MeshAsset* Convert(aiMesh *aimesh){
	MeshAsset *mesh = new MeshAsset(string(aimesh->mName.C_Str()));

	mesh->HasNormals(aimesh->HasNormals());
	mesh->HasTangents(aimesh->HasTangentsAndBitangents());
	mesh->HasBitangents(aimesh->HasTangentsAndBitangents());
	mesh->HasColors(aimesh->HasVertexColors(0));
	mesh->HasBones(aimesh->HasBones());
	for (int k = 0; k < 8; k++)
		mesh->HasTexcoords(k, aimesh->HasTextureCoords(k));

	mesh->VertexCount(aimesh->mNumVertices);

	for (uint32_t i = 0; i < aimesh->mNumVertices; i++) {
		mesh->GetVertices()[i] = ai2dx(aimesh->mVertices[i]);
		if (aimesh->HasNormals())
			mesh->GetNormals()[i] = ai2dx(aimesh->mNormals[i]);
		if (aimesh->HasTangentsAndBitangents()) {
			mesh->GetTangents()[i] = ai2dx(aimesh->mTangents[i]);
			mesh->GetBitangents()[i] = ai2dx(aimesh->mBitangents[i]);
		}

		if (aimesh->HasVertexColors(0))
			mesh->GetColors()[i] = ai2dx(aimesh->mColors[0][i]);

		for (int k = 0; k < 8; k++)
			if (aimesh->HasTextureCoords(k)) {
				XMFLOAT3 v = ai2dx(aimesh->mTextureCoords[k][i]);
				mesh->GetTexcoords(k)[i].x = v.x;
				mesh->GetTexcoords(k)[i].y = v.y;
				mesh->GetTexcoords(k)[i].z = v.z;
				mesh->GetTexcoords(k)[i].w = 0;
			}
	}

	for (uint32_t i = 0; i < aimesh->mNumFaces; i++) {
		const struct aiFace *face = &aimesh->mFaces[i];
		for (uint32_t j = 2; j < face->mNumIndices; j++)
			mesh->AddTriangle(face->mIndices[j], face->mIndices[j - 1], face->mIndices[j - 2]);
	}

	if (aimesh->HasBones()) {
		XMINT4*   bi = mesh->GetBlendIndices();
		XMFLOAT4* bw = mesh->GetBlendWeights();

		for (uint32_t j = 0; j < aimesh->mNumBones; j++) {
			aiBone *bone = aimesh->mBones[j];

			MeshAsset::Bone b(string(bone->mName.C_Str()), ai2dx(bone->mOffsetMatrix));
			for (uint32_t k = 0; k < bone->mNumWeights; k++) {
				aiVertexWeight vweight = bone->mWeights[k];

				// find an unused slot in the vertex's float4
				// or overwrite the lowest-weight bone
				int m = 0;
				for (int l = 0; l < 4; l++) {
					if (XMGET(bw[vweight.mVertexId], l) == 0) {
						XMSET(bi[vweight.mVertexId], l, j);
						XMSET(bw[vweight.mVertexId], l, vweight.mWeight);
						m = -1;
						break;
					}
					if (XMGET(bw[vweight.mVertexId], l) < XMGET(bw[vweight.mVertexId], m))
						m = l;
				}
				if (m != -1) {
					XMSET(bi[vweight.mVertexId], m, j);
					XMSET(bw[vweight.mVertexId], m, vweight.mWeight);
				}
			}
		}
	}

	return mesh;
}

Asset** MeshImporter::Import(string path, int &count) {
	const aiScene *scene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
	string fname = GetNameExt(path);

	count = 0;
	if (scene->HasMeshes()) count += scene->mNumMeshes;
	//if (scene->HasAnimations()) count += scene->mNumAnimations;

	Asset** assets = new Asset*[count];

	if (scene->HasMeshes()) {
		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* aimesh = scene->mMeshes[i];
			if (AssetImporter::verbose)
				printf("%s: %s (%d vertices, %d faces)\n", fname.c_str(), aimesh->mName.C_Str(), aimesh->mNumVertices, aimesh->mNumFaces);
			assets[i] = Convert(aimesh);
		}
	}
	//if (scene->HasAnimations()) {
	//	for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
	//		aiAnimation aianim = scene->mAnimations[i];
	//	}
	//}

	/*
	// load scene information
	stack<aiNode*> nodes;
	nodes.push(scene->mRootNode);
	while (!nodes.empty()) {
		aiNode* node = nodes.top();
		nodes.pop();
		for (unsigned int i = 0; i < node->mNumChildren; i++)
			nodes.push(node->mChildren[i]);

		// add transform information from node
	}
	*/

	aiReleaseImport(scene);

	return assets;
}