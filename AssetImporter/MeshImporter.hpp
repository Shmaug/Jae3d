#pragma once

#include <jstring.hpp>
#include <jvector.hpp>
#include <Asset.hpp>
#include "ImportCommon.hpp"

void ImportMesh(jwstring file, jvector<AssetMetadata> &meta);
Asset** ImportMesh(jwstring file, int &count);
