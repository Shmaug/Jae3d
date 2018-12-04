#pragma once

#include "../Common/jstring.hpp"

class ShaderAsset;

class ShaderImporter {
public:
	static ShaderAsset* CompileShader(jstring path);
};

