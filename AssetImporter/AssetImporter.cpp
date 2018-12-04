#include "AssetImporter.hpp"

#include <comdef.h>

#include "MeshImporter.hpp"
#include "TextureImporter.hpp"
#include "ShaderImporter.hpp"

#include "../Common/IOUtil.hpp"
#include "../Common/Asset.hpp"
#include "../Common/MeshAsset.hpp"
#include "../Common/TextureAsset.hpp"
#include "../Common/ShaderAsset.hpp"

using namespace std;
using namespace DirectX;

bool AssetImporter::verbose = false;

Asset** AssetImporter::ImportObj(jstring path, int &count) {
	return MeshImporter::Import(path, count);
}
Asset** AssetImporter::ImportFbx(jstring path, int &count) {
	return MeshImporter::Import(path, count);
}
Asset** AssetImporter::ImportBlend(jstring path, int &count) {
	return MeshImporter::Import(path, count);
}

TextureAsset* AssetImporter::ImportPng(jstring path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportGif(jstring path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportBmp(jstring path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportTif(jstring path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportJpg(jstring path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportTga(jstring path) {
	return TextureImporter::ImportTGA(path);;
}
TextureAsset* AssetImporter::ImportDDS(jstring path) {
	return TextureImporter::ImportDDS(path);;
}

ShaderAsset* AssetImporter::ImportShader(jstring path) {
	ShaderAsset* shader = new ShaderAsset(GetName(path));

	ShaderAsset::SHADERSTAGE stage;
	jstring end = GetName(path).substr(-3);
	if (end == "_rs")
		stage = ShaderAsset::SHADERSTAGE_ROOTSIG;
	else if (end == "_vs")
		stage = ShaderAsset::SHADERSTAGE_VERTEX;
	else if (end == "_hs")
		stage = ShaderAsset::SHADERSTAGE_HULL;
	else if (end == "_ds")
		stage = ShaderAsset::SHADERSTAGE_DOMAIN;
	else if (end == "_gs")
		stage = ShaderAsset::SHADERSTAGE_GEOMETRY;
	else if (end == "_ps")
		stage = ShaderAsset::SHADERSTAGE_PIXEL;
	else if (end == "_cs")
		stage = ShaderAsset::SHADERSTAGE_COMPUTE;

	shader->ReadShaderStage(utf8toUtf16(path), stage);
	return shader;
}
ShaderAsset* AssetImporter::CompileShader(jstring path) {
	return ShaderImporter::CompileShader(path);
}