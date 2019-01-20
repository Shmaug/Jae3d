#pragma once

#include <Shader.hpp>
#include <jvector.hpp>
#include "ImportCommon.hpp"

void CompileShader(jwstring path, jvector<AssetMetadata> &meta, jvector<jstring> includePaths);
Shader* CompileShader(jwstring path, jvector<jstring> includePaths);