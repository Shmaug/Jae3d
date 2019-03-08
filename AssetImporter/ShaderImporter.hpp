#pragma once

#include <Shader.hpp>
#include <jvector.hpp>
#include "ImportCommon.hpp"

void CompileShader(const jwstring& path, jvector<AssetMetadata>& meta, const jvector<jstring>& includePaths);
Shader* CompileShader(const jwstring& path, const jvector<jstring>& includePaths);