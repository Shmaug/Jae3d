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

void ParseShader(ShaderAsset* shader, string path, int &rootParamIndex) {
	wstring wpath = utf8toUtf16(path);
	ifstream infile(path.c_str());
	string line;
	while (getline(infile, line)) {
		int mode = -1;
		ShaderAsset::SHADERSTAGE stage;
		ShaderAsset::PARAM_TYPE paramType;

		// scan for whitespace-delimited words
		const char *linec = line.c_str();
		for (int i = 0; i < line.size(); i++) {
			#pragma region get next word
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
			#pragma endregion

			if (mode == -1) {
				mode = 0;
				// first word found on the line
				if (word != "#pragma") {
					if (word == "#include")
						mode = -2;
					else
						break;
				}
			}

			switch (mode) {
			case -2: // parse pragmas from #included file
			{
				string dir = path;
				const size_t last_slash_idx = path.rfind('\\');
				if (std::string::npos != last_slash_idx)
					dir = path.substr(0, last_slash_idx + 1);
				ParseShader(shader, GetFullPath(dir + word.substr(1, word.length() - 2)), rootParamIndex);
				break;
			}
			case 0: // reading pragma type
			{
				if (word == "rootsig") {
					stage = ShaderAsset::SHADERSTAGE_ROOTSIG;
					mode = 1;
				} else if (word == "vertex") {
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
				} else if (word == "Parameter") {
					mode = 2;
				}
				break;
			}
			case 1: // reading entry point
			{
				mode = 0;
				HRESULT hr = shader->CompileShaderStage(wpath, word, stage);
				if (FAILED(hr)) {
					_com_error err(hr);
					printf("%s\n", err.ErrorMessage());
				}
				break;
			}
			case 2: // reading parameter type
			{
				if (word == "cbv")
					paramType = ShaderAsset::PARAM_TYPE_CBV;
				else if (word == "srv")
					paramType = ShaderAsset::PARAM_TYPE_SRV;
				else if (word == "uav")
					paramType = ShaderAsset::PARAM_TYPE_UAV;
				else if (word == "samp")
					paramType = ShaderAsset::PARAM_TYPE_SAMPLER;
				else
					assert(false);
				mode = 3;
				break;
			}
			case 3: // reading parameter name
				shader->AddParameter(word, ShaderAsset::Parameter(paramType, rootParamIndex++));
				break;
			}
		}
	}
}

ShaderAsset* AssetImporter::ImportShader(string path) {
	ShaderAsset* shader = new ShaderAsset(GetName(path));

	ShaderAsset::SHADERSTAGE stage;
	string end = GetName(path).substr(-3);
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
ShaderAsset* AssetImporter::CompileShader(string path) {
	ShaderAsset* shader = new ShaderAsset(GetName(path));

	int rootParamIndex = 0;

	ParseShader(shader, path, rootParamIndex);

	return shader;
}