#pragma once

#include <Texture.hpp>
#include <jvector.hpp>
#include "ImportCommon.hpp"

// Imports a texture and metadata and appends to meta
void ImportTexture(jwstring file, jvector<AssetMetadata> &meta);
// Imports a texture with settings in a file [path].meta
Texture* ImportTexture(jwstring file);
// Imports a texture with the settings in metadata
Texture* ImportTexture(jwstring file, AssetMetadata &metadata);