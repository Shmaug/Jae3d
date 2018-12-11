#pragma once

#include <Shader.hpp>

Shader* CompileShader(jwstring path);
// Reads a precompiled shader
Shader* ReadShader(jwstring path);
