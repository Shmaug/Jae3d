#pragma once

#include "../Common/jstring.hpp"

class Asset;
class TextureAsset;
class ShaderAsset;

class AssetImporter {
public:
	static bool verbose;

	static Asset** ImportObj(jstring file, int &count);
	static Asset** ImportFbx(jstring file, int &count);
	static Asset** ImportBlend(jstring file, int &count);
	
	static TextureAsset* ImportPng(jstring file);
	static TextureAsset* ImportGif(jstring file);
	static TextureAsset* ImportBmp(jstring file);
	static TextureAsset* ImportTif(jstring file);
	static TextureAsset* ImportJpg(jstring file);
	static TextureAsset* ImportDDS(jstring file);
	static TextureAsset* ImportTga(jstring file);
		
	static ShaderAsset* ImportShader(jstring file);
	static ShaderAsset* CompileShader(jstring file);
};

