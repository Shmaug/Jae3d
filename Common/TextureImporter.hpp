#pragma once

#include <string>

class TextureAsset;

class TextureImporter {
public:
	static TextureAsset* Import(std::string file);
	static TextureAsset* ImportTGA(std::string file);
	static TextureAsset* ImportDDS(std::string file);
};

