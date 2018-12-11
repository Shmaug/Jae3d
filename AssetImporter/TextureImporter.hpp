#pragma once

#include <Texture.hpp>

struct TexMeta {
	DXGI_FORMAT format;
	bool linear;
	bool mipMaps;
	ALPHA_MODE alphaMode;
};

Texture* ImportTexture(jwstring file);
Texture* ImportTexture(jwstring file, TexMeta &metadata);