#pragma once

#include <Shader.hpp>
#include <jvector.hpp>
#include "ImportCommon.hpp"

void CompileShader(jwstring path, jvector<AssetMetadata> &meta);
void ReadShader(jwstring path, jvector<AssetMetadata> &meta);

Shader* CompileShader(jwstring path);
// Reads a precompiled shader
Shader* ReadShader(jwstring path);
