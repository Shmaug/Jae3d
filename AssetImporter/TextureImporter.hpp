#pragma once

#include <Texture.hpp>
#include <jvector.hpp>
#include "ImportCommon.hpp"

void ImportTexture(jwstring file, jvector<AssetMetadata> &meta);
Texture* ImportTexture(jwstring file);
Texture* ImportTexture(jwstring file, AssetMetadata &metadata);