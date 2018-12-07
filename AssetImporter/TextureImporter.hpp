#pragma once

#include <Texture.hpp>

class TextureImporter {
public:
	struct meta {
		DXGI_FORMAT format;
		bool linear;
		bool mipMaps;
	};

	static Texture* Import(jwstring file);
	static Texture* Import(jwstring file, meta &metadata);
	static Texture* ImportDDS(jwstring file);
};

