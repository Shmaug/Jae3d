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
#include "../Common/MeshAsset.hpp"
#include "../Common/IOUtil.hpp"

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
void XMSET(XMUINT4 &v, int i, unsigned int val) {
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

	mesh->HasSemantic(MeshAsset::SEMANTIC_NORMAL, aimesh->HasNormals());
	mesh->HasSemantic(MeshAsset::SEMANTIC_TANGENT, aimesh->HasTangentsAndBitangents());
	mesh->HasSemantic(MeshAsset::SEMANTIC_BINORMAL, aimesh->HasTangentsAndBitangents());
	mesh->HasSemantic(MeshAsset::SEMANTIC_COLOR0, aimesh->HasVertexColors(0));
	mesh->HasSemantic(MeshAsset::SEMANTIC_COLOR1, aimesh->HasVertexColors(1));
	mesh->HasSemantic(MeshAsset::SEMANTIC_BLENDINDICES, aimesh->HasBones());
	mesh->HasSemantic(MeshAsset::SEMANTIC_BLENDWEIGHT, aimesh->HasBones());
	mesh->HasSemantic(MeshAsset::SEMANTIC_TEXCOORD0, aimesh->HasTextureCoords(0));
	mesh->HasSemantic(MeshAsset::SEMANTIC_TEXCOORD1, aimesh->HasTextureCoords(1));
	mesh->HasSemantic(MeshAsset::SEMANTIC_TEXCOORD2, aimesh->HasTextureCoords(2));
	mesh->HasSemantic(MeshAsset::SEMANTIC_TEXCOORD3, aimesh->HasTextureCoords(3));

	mesh->VertexCount(aimesh->mNumVertices);

	for (unsigned int i = 0; i < aimesh->mNumVertices; i++) {
		mesh->GetVertices()[i] = ai2dx(aimesh->mVertices[i]);
		if (aimesh->HasNormals())
			mesh->GetNormals()[i] = ai2dx(aimesh->mNormals[i]);
		if (aimesh->HasTangentsAndBitangents()) {
			mesh->GetTangents()[i] = ai2dx(aimesh->mTangents[i]);
			mesh->GetBinormals()[i] = ai2dx(aimesh->mBitangents[i]);
		}

		for (int k = 0; k < 2; k++)
			if (aimesh->HasVertexColors(k))
				mesh->GetColors(k)[i] = ai2dx(aimesh->mColors[k][i]);

		for (int k = 0; k < 4; k++)
			if (aimesh->HasTextureCoords(k)) {
				XMFLOAT3 v = ai2dx(aimesh->mTextureCoords[k][i]);
				mesh->GetTexcoords(k)[i].x = v.x;
				mesh->GetTexcoords(k)[i].y = v.y;
				mesh->GetTexcoords(k)[i].z = v.z;
				mesh->GetTexcoords(k)[i].w = 0;
			}
	}

	for (unsigned int i = 0; i < aimesh->mNumFaces; i++) {
		const struct aiFace *face = &aimesh->mFaces[i];
		for (unsigned int j = 2; j < face->mNumIndices; j++)
			mesh->AddTriangle(face->mIndices[j], face->mIndices[j - 1], face->mIndices[j - 2]);
	}

	if (aimesh->HasBones()) {
		XMUINT4*   bi = mesh->GetBlendIndices();
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
	const aiScene *scene = aiImportFile(path.c_str(), 
		aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_MakeLeftHanded | aiProcess_FlipUVs);
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