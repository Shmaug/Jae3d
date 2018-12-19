#pragma once

#include <Shader.hpp>
#include <jvector.hpp>
#include "ImportCommon.hpp"

void CompileShader(jwstring path, jvector<AssetMetadata> &meta, jvector<jwstring> includePaths);
void ReadShader(jwstring path, jvector<AssetMetadata> &meta);

Shader* CompileShader(jwstring path, jvector<jwstring> includePaths);
// Reads a precompiled shader
Shader* ReadShader(jwstring path);
