#include "AssetImporter.hpp"

#include <stack>

#include <d3d11.h>
#include <d3d12.h>
#include <DirectXMath.h>

#include <comdef.h>
#include <DirectXTex.h>

#include <iostream>
#include <sstream>
#include <iterator>

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

#include "Util.hpp"
#include "Asset.hpp"
#include "MeshAsset.hpp"
#include "TextureAsset.hpp"
#include "ShaderAsset.hpp"

using namespace std;
using namespace DirectX;

bool AssetImporter::verbose = false;

std::vector<Asset*> AssetImporter::assets;

wstring utf8toUtf16(const string &str) {
	if (str.empty())
		return wstring();

	size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	vector<wchar_t> buffer(charsNeeded);
	int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &buffer[0], (int)buffer.size());
	if (charsConverted == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	return wstring(&buffer[0], charsConverted);
}

string GetName(string path) {
	char const *str = path.c_str();

	int f = 0;
	int l = 0;
	for (int i = 0; i < path.length(); i++) {
		if (str[i] == '\\')
			f = i + 1;
		else if (str[i] == '.')
			l = i;
	}

	return path.substr(f, l - f);
}

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

void Import3d(string path) {
	const aiScene *scene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

	stack<aiNode*> nodes;
	nodes.push(scene->mRootNode);
	while (!nodes.empty()) {
		aiNode* n = nodes.top();
		nodes.pop();

		if (AssetImporter::verbose)
			printf("   aiNode %s\n", n->mName.C_Str());

		for (unsigned int i = 0; i < n->mNumMeshes; i++) {
			const struct aiMesh* aimesh = scene->mMeshes[n->mMeshes[i]];

			if (AssetImporter::verbose)
				printf("      aiMesh%s (%d vertices, %d faces)\n", aimesh->mName.C_Str(), aimesh->mNumVertices, aimesh->mNumFaces);

			MeshAsset *mesh = new MeshAsset(string(aimesh->mName.C_Str()));
			AssetImporter::assets.push_back((Asset*)mesh);
			
			for (uint32_t j = 0; j < aimesh->mNumVertices; j++) {
				mesh->m_Vertices.push_back(ai2dx(aimesh->mVertices[j]));
				if (aimesh->HasNormals())
					mesh->m_Normals.push_back(ai2dx(aimesh->mNormals[j]));
				if (aimesh->HasTangentsAndBitangents()) {
					mesh->m_Tangents.push_back(ai2dx(aimesh->mTangents[j]));
					mesh->m_Bitangents.push_back(ai2dx(aimesh->mBitangents[j]));
				}
				if (aimesh->HasVertexColors(0))
					mesh->m_Colors.push_back(ai2dx(aimesh->mColors[0][j]));

				// allocate space for bone data
				if (aimesh->HasBones()) {
					mesh->m_BlendIndices.push_back({ 0, 0, 0, 0});
					mesh->m_BlendWeights.push_back({ 0, 0, 0, 0 });
				}
				for (int k = 0; k < 8; k++)
					if (aimesh->HasTextureCoords(k))
						mesh->m_Texcoords[k].push_back(ai2dx(aimesh->mTextureCoords[k][j]));
			}

			for (uint32_t j = 0; j < aimesh->mNumFaces; j++) {
				const struct aiFace *face = &aimesh->mFaces[j];
				for (uint32_t k = 2; k < face->mNumIndices; k++) {
					mesh->AddTriangle(face->mIndices[k], face->mIndices[k - 1], face->mIndices[k - 2]);
				}
			}

			if (aimesh->HasBones()) {
				for (uint32_t j = 0; j < aimesh->mNumBones; j++) {
					aiBone *bone = aimesh->mBones[j];

					MeshAsset::Bone b;
					b.m_Name = string(bone->mName.C_Str());
					b.m_BoneToMesh = ai2dx(bone->mOffsetMatrix);
					for (uint32_t k = 0; k < bone->mNumWeights; k++) {
						aiVertexWeight vweight = bone->mWeights[k];

						// find an unused slot in the vertex's float4
						// or overwrite the lowest-weight bone
						int m = 0;
						for (int l = 0; l < 4; l++) {
							if (XMGET(mesh->m_BlendWeights[vweight.mVertexId], l) == 0) {
								XMSET(mesh->m_BlendIndices[vweight.mVertexId], l,  j);
								XMSET(mesh->m_BlendWeights[vweight.mVertexId], l,  vweight.mWeight);
								m = -1;
								break;
							}
							if (XMGET(mesh->m_BlendWeights[vweight.mVertexId], l) < XMGET(mesh->m_BlendWeights[vweight.mVertexId], m))
								m = l;
						}
						if (m != -1) {
							XMSET(mesh->m_BlendIndices[vweight.mVertexId], m, j);
							XMSET(mesh->m_BlendWeights[vweight.mVertexId], m, vweight.mWeight);
						}
					}
				}
			}
		}

		for (unsigned int i = 0; i < n->mNumChildren; i++)
			nodes.push(n->mChildren[i]);
	}

	aiReleaseImport(scene);
}

const char* formatToString(DXGI_FORMAT f) {
	static const char* formats[120] = {
		"UNKNOWN",
		"R32G32B32A32_TYPELESS",
		"R32G32B32A32_FLOAT",
		"R32G32B32A32_UINT",
		"R32G32B32A32_SINT",
		"R32G32B32_TYPELESS",
		"R32G32B32_FLOAT",
		"R32G32B32_UINT",
		"R32G32B32_SINT",
		"R16G16B16A16_TYPELESS",
		"R16G16B16A16_FLOAT",
		"R16G16B16A16_UNORM",
		"R16G16B16A16_UINT",
		"R16G16B16A16_SNORM",
		"R16G16B16A16_SINT",
		"R32G32_TYPELESS",
		"R32G32_FLOAT",
		"R32G32_UINT",
		"R32G32_SINT",
		"R32G8X24_TYPELESS",
		"D32_FLOAT_S8X24_UINT",
		"R32_FLOAT_X8X24_TYPELESS",
		"X32_TYPELESS_G8X24_UINT",
		"R10G10B10A2_TYPELESS",
		"R10G10B10A2_UNORM",
		"R10G10B10A2_UINT",
		"R11G11B10_FLOAT",
		"R8G8B8A8_TYPELESS",
		"R8G8B8A8_UNORM",
		"R8G8B8A8_UNORM_SRGB",
		"R8G8B8A8_UINT",
		"R8G8B8A8_SNORM",
		"R8G8B8A8_SINT",
		"R16G16_TYPELESS",
		"R16G16_FLOAT",
		"R16G16_UNORM",
		"R16G16_UINT",
		"R16G16_SNORM",
		"R16G16_SINT",
		"R32_TYPELESS",
		"D32_FLOAT",
		"R32_FLOAT",
		"R32_UINT",
		"R32_SINT",
		"R24G8_TYPELESS",
		"D24_UNORM_S8_UINT",
		"R24_UNORM_X8_TYPELESS",
		"X24_TYPELESS_G8_UINT",
		"R8G8_TYPELESS",
		"R8G8_UNORM",
		"R8G8_UINT",
		"R8G8_SNORM",
		"R8G8_SINT",
		"R16_TYPELESS",
		"R16_FLOAT",
		"D16_UNORM",
		"R16_UNORM",
		"R16_UINT",
		"R16_SNORM",
		"R16_SINT",
		"R8_TYPELESS",
		"R8_UNORM",
		"R8_UINT",
		"R8_SNORM",
		"R8_SINT",
		"A8_UNORM",
		"R1_UNORM",
		"R9G9B9E5_SHAREDEXP",
		"R8G8_B8G8_UNORM",
		"G8R8_G8B8_UNORM",
		"BC1_TYPELESS",
		"BC1_UNORM",
		"BC1_UNORM_SRGB",
		"BC2_TYPELESS",
		"BC2_UNORM",
		"BC2_UNORM_SRGB",
		"BC3_TYPELESS",
		"BC3_UNORM",
		"BC3_UNORM_SRGB",
		"BC4_TYPELESS",
		"BC4_UNORM",
		"BC4_SNORM",
		"BC5_TYPELESS",
		"BC5_UNORM",
		"BC5_SNORM",
		"B5G6R5_UNORM",
		"B5G5R5A1_UNORM",
		"B8G8R8A8_UNORM",
		"B8G8R8X8_UNORM",
		"R10G10B10_XR_BIAS_A2_UNORM",
		"B8G8R8A8_TYPELESS",
		"B8G8R8A8_UNORM_SRGB",
		"B8G8R8X8_TYPELESS",
		"B8G8R8X8_UNORM_SRGB",
		"BC6H_TYPELESS",
		"BC6H_UF16",
		"BC6H_SF16",
		"BC7_TYPELESS",
		"BC7_UNORM",
		"BC7_UNORM_SRGB",
		"AYUV",
		"Y410",
		"Y416",
		"NV12",
		"P010",
		"P016",
		"420_OPAQUE",
		"YUY2",
		"Y210",
		"Y216",
		"NV11",
		"AI44",
		"IA44",
		"P8",
		"A8P8",
		"B4G4R4A4_UNORM",
		"P208",
		"V208",
		"V408",
		"FORCE_UINT"
	};
	int i = (int)f;
	if (i == 0xffffffff)
		return formats[119];
	if (i >= 130) i -= 14;
	return formats[i];
}
Asset* Convert(ScratchImage &image, TexMetadata *info, string name) {
	if (AssetImporter::verbose)
		printf("   %dD %s %dx%d, %d slice(s) %d mip levels\n", (int)info->dimension - 1, formatToString(info->format), (int)info->width, (int)info->height, (int)info->depth, (int)info->mipLevels);
	
	TextureAsset *asset = new TextureAsset(name, (int)info->width, (int)info->height, (int)info->dimension - 1, info->format, (int)info->mipLevels);
	AssetImporter::assets.push_back((Asset*)asset);

	uint8_t *pixels = image.GetPixels();
	asset->SetPixels(pixels, image.GetPixelsSize());

	return asset;
}
void ImportWIC(string path) {
	auto image = make_unique<ScratchImage>();
	TexMetadata info;
	HRESULT hr = LoadFromWICFile(utf8toUtf16(path).c_str(), WIC_FLAGS_ALL_FRAMES, &info, *image);
	if (FAILED(hr)) {
		_com_error err(hr);
		printf("Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
		return;
	}
	Convert(*image, &info, GetName(path));
}

void AssetImporter::ImportObj(string path) {
	return Import3d(path);
}
void AssetImporter::ImportFbx(string path) {
	return Import3d(path);
}
void AssetImporter::ImportBlend(string path) {
	return Import3d(path);
}

void AssetImporter::ImportPng(string path) {
	return ImportWIC(path);
}
void AssetImporter::ImportGif(string path) {
	return ImportWIC(path);
}
void AssetImporter::ImportBmp(string path) {
	return ImportWIC(path);
}
void AssetImporter::ImportTif(string path) {
	return ImportWIC(path);
}
void AssetImporter::ImportJpg(string path) {
	return ImportWIC(path);
}
void AssetImporter::ImportTga(string path) {
	auto image = make_unique<ScratchImage>();
	TexMetadata info;
	HRESULT hr = LoadFromTGAFile(utf8toUtf16(path).c_str(), &info, *image);
	if (FAILED(hr)) {
		_com_error err(hr);
		printf("Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
	}
	Convert(*image, &info, GetName(path));
}
void AssetImporter::ImportDds(string path) {
	auto image = make_unique<ScratchImage>();
	TexMetadata info;
	HRESULT hr = LoadFromDDSFile(utf8toUtf16(path).c_str(), DDS_FLAGS_NONE, &info, *image);
	if (FAILED(hr)) {
		_com_error err(hr);
		printf("Failed to read %s: %s\n", path.c_str(), err.ErrorMessage());
	}
	Convert(*image, &info, GetName(path));
}

void AssetImporter::ImportShader(string path) {
	ShaderAsset* shader = new ShaderAsset(GetName(path));
	assets.push_back((Asset*)shader);

	ShaderAsset::ShaderStage stage;
	string end = GetName(path).substr(-3);
	if (end == "_vs")
		stage = ShaderAsset::ShaderStage::Vertex;
	else if (end == "_hs")
		stage = ShaderAsset::ShaderStage::Hull;
	else if (end == "_ds")
		stage = ShaderAsset::ShaderStage::Domain;
	else if (end == "_gs")
		stage = ShaderAsset::ShaderStage::Geometry;
	else if (end == "_ps")
		stage = ShaderAsset::ShaderStage::Pixel;
	else if (end == "_cs")
		stage = ShaderAsset::ShaderStage::Compute;

	shader->ReadShaderStage(utf8toUtf16(path), stage);
}
void AssetImporter::CompileShader(string path) {
	ShaderAsset* shader = new ShaderAsset(GetName(path));
	assets.push_back((Asset*)shader);

	wstring wpath = utf8toUtf16(path);

	ifstream infile(path.c_str());
	string line;
	while (getline(infile, line)) {
		int mode = -1;
		ShaderAsset::ShaderStage stage;

		// scan for whitespace delimited words
		const char *linec = line.c_str();
		for (int i = 0; i < line.size(); i++) {
			// scan to the next word
			while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
				i++;
			if (i >= line.size()) break;

			// scan to the end of the word
			string word = "";
			int j = i;
			while (j < line.size() && line[j] != ' ' && line[j] != '\t') {
				word += line[j];
				j++;
			}
			i = j;

			// first word found on the line must be //pragma to continue
			if (mode == -1) {
				if (word != "//pragma")
					break;
				mode = 0;
				continue;
			}

			if (mode == 0) {
				if (word == "vertex") {
					stage = ShaderAsset::ShaderStage::Vertex;
					mode = 1;
				} else if (word == "hull") {
					stage = ShaderAsset::ShaderStage::Hull;
					mode = 1;
				} else if (word == "domain") {
					stage = ShaderAsset::ShaderStage::Domain;
					mode = 1;
				} else if (word == "geometry") {
					stage = ShaderAsset::ShaderStage::Geometry;
					mode = 1;
				} else if (word == "pixel") {
					stage = ShaderAsset::ShaderStage::Pixel;
					mode = 1;
				} else if (word == "compute") {
					stage = ShaderAsset::ShaderStage::Compute;
					mode = 1;
				}
			} else if (mode == 1) {
				mode = 0;
				HRESULT hr = shader->CompileShaderStage(wpath, word, stage);
				if (FAILED(hr)) {
					_com_error err(hr);
					printf("%s\n", err.ErrorMessage());
				}
			}
		}
	}
}

void AssetImporter::Write(const char *file) {
	// ASSET file format
	// OFFSET	TYPE		VALUE
	// --------------------------------------
	// 0		uint64_t	14242
	// 8		5 bytes		ASSET
	// 13		uint64_t	version number
	// 21		uint32_t	asset count
	// 25-?		HEADER array
	// ?-eof	DATA array

	// HEADER spec
	// OFFSET	TYPE		VALUE
	// --------------------------------------
	// 0		uint8_t		HEADER identifier
	// 4		uint64_t	Data Offset (from file start)
	// 12		uint64_t	Header Size
	// 20		uint64_t	Data Size
	// 28		string		Name

	ofstream fs;
	fs.open(file, ios::out | ios::binary);
	if (!fs.is_open()) {
		perror("Failed to open file for reading!");
		return;
	}

	printf("Writing to %s\n", file);

	WriteStream(fs, (uint64_t)14242);	// 'magic' number
	fs.write("ASSET", 5);
	WriteStream(fs, (uint64_t)1);		// version number

	WriteStream(fs, (uint32_t)assets.size());
	for (int i = 0; i < assets.size(); i++) {
		uint64_t p = fs.tellp();
		assets[i]->WriteHeader(fs);

		// Write the size of the header in the appropriate place
		uint64_t q = fs.tellp();
		fs.seekp(p + sizeof(uint8_t) + sizeof(uint64_t));
		WriteStream(fs, q - p);
		fs.seekp(q);
	}

	for (int i = 0; i < assets.size(); i++) {
		uint64_t p = fs.tellp();
		assets[i]->WriteData(fs);

		uint64_t q = fs.tellp();
		fs.seekp(assets[i]->m_HeaderPositionPos + sizeof(uint8_t));
		WriteStream(fs, p); // data pos
		fs.seekp(assets[i]->m_HeaderPositionPos + sizeof(uint8_t) + 2 * sizeof(uint64_t));
		WriteStream(fs, q - p); // data size
		fs.seekp(q);
	}

	fs.close();
}
bool AssetImporter::Validate(const char *file) {
	ifstream fs;
	fs.open(file, ios::in | ios::binary);

	#define FAIL(c, msg) if (c) { fs.close(); printf(msg); return false; }
	#define FAILF(c, msg, fmt) if (c) { fs.close(); printf(msg, fmt); return false; }

	printf("Validating %s\n", file);

	// magic number
	uint64_t mn = ReadStream<uint64_t>(fs);
	FAILF(mn != (uint64_t)14242, "Incorrect magic number! (%lu)\n", (unsigned long)mn);
	// ASSET text
	char am[5];
	fs.read(am, 5);
	FAILF(!strcmp(am, "ASSET"), "No ASSET text! (%s)\n", am);

	uint64_t version = ReadStream<uint64_t>(fs);
	uint32_t assetCount = ReadStream<uint32_t>(fs);

	FAILF(assetCount != assets.size(), "Incorrect asset count! (%d)\n", assetCount);

	vector<uint64_t> datapos((size_t)assetCount);
	vector<uint64_t> headerpos((size_t)assetCount);

	for (uint32_t i = 0; i < assetCount; i++) {
		headerpos[i] = (int64_t)fs.tellg();

		Asset::AssetHeader h(fs);
		printf("%lu Header %s (%lu bytes at %lu)\n", (unsigned long)headerpos[i], h.name.c_str(), (unsigned long)h.dataSize, (unsigned long)h.dataPos);
		FAILF(h.id != 0x01, "Incorrect header identifier! (%d)\n", h.id);
		FAIL(h.name != assets[i]->m_Name, "Incorrect header name!\n");
		
		datapos[i] = h.dataPos;

		fs.seekg(headerpos[i] + h.headerSize);
	}

	for (uint32_t i = 0; i < assetCount; i++) {
		datapos[i] = (uint64_t)fs.tellg();

		Asset::AssetData d(fs);
		printf("%lu Data %s (%lu bytes)\n", (unsigned long)datapos[i], d.name.c_str(), (unsigned long)d.dataSize);
		FAILF(d.id != 0x02, "Incorrect data identifier! (%d)\n", d.id);
		FAIL(d.name != assets[i]->m_Name, "Incorrect data name!\n");

		fs.seekg(datapos[i] + d.dataSize);
	}

	#undef FAIL
	#undef FAILF

	fs.close();
	return true;
}
void AssetImporter::CleanUp() {
	for (int i = 0; i < assets.size(); i++)
		delete assets[i];
	assets.clear();
}