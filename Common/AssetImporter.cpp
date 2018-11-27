#include "AssetImporter.hpp"

#include <comdef.h>

#include "MeshImporter.hpp"
#include "TextureImporter.hpp"

#include "IOUtil.hpp"
#include "Asset.hpp"
#include "MeshAsset.hpp"
#include "TextureAsset.hpp"
#include "ShaderAsset.hpp"

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
	return shader;
}
ShaderAsset* AssetImporter::CompileShader(string path) {
	ShaderAsset* shader = new ShaderAsset(GetName(path));

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
	return shader;
}