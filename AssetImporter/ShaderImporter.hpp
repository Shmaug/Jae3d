#pragma once

#include <Shader.hpp>

class ShaderImporter {
public:
	static Shader* CompileShader(jwstring path);
	// Reads a precompiled shader
	static Shader* ReadShader(jwstring path);
};

