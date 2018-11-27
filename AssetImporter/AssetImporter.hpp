#pragma once

#include <iostream>
#include <string>
#include <vector>

class Asset;
class TextureAsset;
class ShaderAsset;

class AssetImporter {
public:
	static bool verbose;

	static Asset** ImportObj(std::string file, int &count);
	static Asset** ImportFbx(std::string file, int &count);
	static Asset** ImportBlend(std::string file, int &count);
	
	static TextureAsset* ImportPng(std::string file);
	static TextureAsset* ImportGif(std::string file);
	static TextureAsset* ImportBmp(std::string file);
	static TextureAsset* ImportTif(std::string file);
	static TextureAsset* ImportJpg(std::string file);
	static TextureAsset* ImportDDS(std::string file);
	static TextureAsset* ImportTga(std::string file);
		
	static ShaderAsset* ImportShader(std::string file);
	static ShaderAsset* CompileShader(std::string file);
};

