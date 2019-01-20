#pragma once

#include <jstring.hpp>
#include <jvector.hpp>
#include <Asset.hpp>
#include "ImportCommon.hpp"

void ImportScene(jwstring file, jvector<AssetMetadata> &meta);
void ImportScene(jwstring file, jvector<Asset*> &assets);
