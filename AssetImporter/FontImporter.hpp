#pragma once

#include <jstring.hpp>
#include <jvector.hpp>
#include <Font.hpp>
#include "ImportCommon.hpp"

void ImportFont(jwstring path, jvector<AssetMetadata> &meta);
Font* ImportFont(jwstring path);
Font* ImportFont(jwstring path, AssetMetadata &metadata);