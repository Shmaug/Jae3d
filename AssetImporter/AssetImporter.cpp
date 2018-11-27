#include "AssetImporter.hpp"

#include <comdef.h>

#include "MeshImporter.hpp"
#include "TextureImporter.hpp"

#include "../Common/IOUtil.hpp"
#include "../Common/Asset.hpp"
#include "../Common/MeshAsset.hpp"
#include "../Common/TextureAsset.hpp"
#include "../Common/ShaderAsset.hpp"

using namespace std;
using namespace DirectX;

bool AssetImporter::verbose = false;

Asset** AssetImporter::ImportObj(string path, int &count) {
	return MeshImporter::Import(path, count);
}
Asset** AssetImporter::ImportFbx(string path, int &count) {
	return MeshImporter::Import(path, count);
}
Asset** AssetImporter::ImportBlend(string path, int &count) {
	return MeshImporter::Import(path, count);
}

TextureAsset* AssetImporter::ImportPng(string path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportGif(string path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportBmp(string path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportTif(string path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportJpg(string path) {
	return TextureImporter::Import(path);;
}
TextureAsset* AssetImporter::ImportTga(string path) {
	return TextureImporter::ImportTGA(path);;
}
TextureAsset* AssetImporter::ImportDDS(string path) {
	return TextureImporter::ImportDDS(path);;
}

ShaderAsset* AssetImporter::ImportShader(string path) {
	ShaderAsset* shader = new ShaderAsset(GetName(path));

	ShaderAsset::SHADERSTAGE stage;
	string end = GetName(path).substr(-3);
	if (end == "_vs")
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
ShaderAsset* AssetImporter::CompileShader(string path) {
	ShaderAsset* shader = new ShaderAsset(GetName(path));

	wstring wpath = utf8toUtf16(path);

	ifstream infile(path.c_str());
	string line;
	while (getline(infile, line)) {
		int mode = -1;
		ShaderAsset::SHADERSTAGE stage;

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
					stage = ShaderAsset::SHADERSTAGE_VERTEX;
					mode = 1;
				} else if (word == "hull") {
					stage = ShaderAsset::SHADERSTAGE_HULL;
					mode = 1;
				} else if (word == "domain") {
					stage = ShaderAsset::SHADERSTAGE_DOMAIN;
					mode = 1;
				} else if (word == "geometry") {
					stage = ShaderAsset::SHADERSTAGE_GEOMETRY;
					mode = 1;
				} else if (word == "pixel") {
					stage = ShaderAsset::SHADERSTAGE_PIXEL;
					mode = 1;
				} else if (word == "compute") {
					stage = ShaderAsset::SHADERSTAGE_COMPUTE;
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
	return shader;
}