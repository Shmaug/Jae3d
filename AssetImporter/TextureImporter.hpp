#pragma once

#include "../Common/jstring.hpp"

class TextureAsset;

class TextureImporter {
public:
	static TextureAsset* Import(jstring file);
	static TextureAsset* ImportTGA(jstring file);
	static TextureAsset* ImportDDS(jstring file);
};

