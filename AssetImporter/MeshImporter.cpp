#include "MeshImporter.hpp"

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

#include <IOUtil.hpp>
#include <Mesh.hpp>

#include <d3d12.h>
#include <DirectXMath.h>

#include <unordered_set>
#include <stack>

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

void Convert(Mesh* mesh, unsigned int submesh, aiMesh *aimesh){
	if (aimesh->HasNormals()) mesh->HasSemantic(MESH_SEMANTIC_NORMAL, true);
	if (aimesh->HasTangentsAndBitangents()) mesh->HasSemantic(MESH_SEMANTIC_TANGENT, true);
	if (aimesh->HasTangentsAndBitangents()) mesh->HasSemantic(MESH_SEMANTIC_BINORMAL, true);
	if (aimesh->HasVertexColors(0)) mesh->HasSemantic(MESH_SEMANTIC_COLOR0, true);
	if (aimesh->HasVertexColors(1)) mesh->HasSemantic(MESH_SEMANTIC_COLOR1, true);
	if (aimesh->HasBones()) {
		mesh->HasSemantic(MESH_SEMANTIC_BLENDINDICES, true);
		mesh->HasSemantic(MESH_SEMANTIC_BLENDWEIGHT, true);
	}
	if (aimesh->HasTextureCoords(0)) mesh->HasSemantic(MESH_SEMANTIC_TEXCOORD0, true);
	if (aimesh->HasTextureCoords(1)) mesh->HasSemantic(MESH_SEMANTIC_TEXCOORD1, true);
	if (aimesh->HasTextureCoords(2)) mesh->HasSemantic(MESH_SEMANTIC_TEXCOORD2, true);
	if (aimesh->HasTextureCoords(3)) mesh->HasSemantic(MESH_SEMANTIC_TEXCOORD3, true);

	unsigned int c = mesh->VertexCount();
	mesh->VertexCount(c + aimesh->mNumVertices);

	for (unsigned int j = 0; j < aimesh->mNumVertices; j++) {
		unsigned int i = j + c;

		mesh->GetVertices()[i] = ai2dx(aimesh->mVertices[j]);

		if (aimesh->HasNormals())
			mesh->GetNormals()[i] = ai2dx(aimesh->mNormals[j]);
		if (aimesh->HasTangentsAndBitangents()) {
			mesh->GetTangents()[i] = ai2dx(aimesh->mTangents[j]);
			mesh->GetBinormals()[i] = ai2dx(aimesh->mBitangents[j]);
		}

		for (int k = 0; k < 2; k++)
			if (aimesh->HasVertexColors(k))
				mesh->GetColors(k)[i] = ai2dx(aimesh->mColors[k][j]);

		for (int k = 0; k < 4; k++)
			if (aimesh->HasTextureCoords(k)) {
				XMFLOAT3 v = ai2dx(aimesh->mTextureCoords[k][j]);
				mesh->GetTexcoords(k)[i].x = v.x;
				mesh->GetTexcoords(k)[i].y = v.y;
				mesh->GetTexcoords(k)[i].z = v.z;
				mesh->GetTexcoords(k)[i].w = 0;
			}
	}

	for (unsigned int i = 0; i < aimesh->mNumFaces; i++) {
		const struct aiFace *face = &aimesh->mFaces[i];
		for (unsigned int j = 2; j < face->mNumIndices; j++)
			mesh->AddTriangle(c + face->mIndices[j], c + face->mIndices[j - 1], c + face->mIndices[j - 2], submesh);
	}

	if (aimesh->HasBones()) {
		XMUINT4*  bi = mesh->GetBlendIndices() + c;
		XMFLOAT4* bw = mesh->GetBlendWeights() + c;

		for (uint32_t j = 0; j < aimesh->mNumBones; j++) {
			aiBone *bone = aimesh->mBones[j];

			MeshBone b(utf8toUtf16(bone->mName.C_Str()), ai2dx(bone->mOffsetMatrix));
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
}

void ImportScene(jwstring path, jvector<AssetMetadata> &meta) {
	AssetMetadata metadata(path);
	jvector<Asset*> assets;
	ImportScene(path, assets);
	for (int i = 0; i < assets.size(); i++) {
		AssetMetadata t = metadata;
		t.asset = std::shared_ptr<Asset>(assets[i]);
		meta.push_back(t);
	}
}
void ImportScene(jwstring path, jvector<Asset*> &assets) {
	const aiScene *scene = aiImportFile(utf16toUtf8(path).c_str(),
		aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_Triangulate |
		aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_GenUVCoords |
		aiProcess_ValidateDataStructure | aiProcess_ImproveCacheLocality |
		aiProcess_MakeLeftHanded | aiProcess_FlipUVs);
	jwstring fname = GetNameExtW(path);

	std::unordered_set<jstring> meshes;

	//if (scene->HasAnimations()) {
	//	for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
	//		aiAnimation aianim = scene->mAnimations[i];
	//	}
	//}

	// load scene information
	std::stack<aiNode*> nodes;
	nodes.push(scene->mRootNode);
	while (!nodes.empty()) {
		aiNode* node = nodes.top();
		nodes.pop();

		if (node->mNumMeshes) {
			Mesh* mesh = new Mesh(utf8toUtf16(node->mName.C_Str()));

			wprintf(L"%S: %d meshes\n", node->mName.C_Str(), node->mNumMeshes);
			for (unsigned int i = 0; i < node->mNumMeshes; i++) {
				aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];

				aiString matname;
				scene->mMaterials[aimesh->mMaterialIndex]->Get(AI_MATKEY_NAME, matname);
				wprintf(L"  %S: %d vertices, %d faces\n", matname.C_Str(), aimesh->mNumVertices, aimesh->mNumFaces);
				Convert(mesh, i, scene->mMeshes[node->mMeshes[i]]);
			}
			assets.push_back(mesh);
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
			nodes.push(node->mChildren[i]);
	}

	aiReleaseImport(scene);
}