#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include <vector>
#include <map>

class RootSignature;

class Shader {
public:
	Shader();
	~Shader();

	std::string name;
	_WRL::ComPtr<ID3DBlob> vertexBlob;
	_WRL::ComPtr<ID3DBlob> pixelBlob;

	RootSignature *m_RootSignature;
	_WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
};

class ShaderLibrary {
private:
	static std::map<std::string, Shader*> shaders;
	static void LoadShader(_WRL::ComPtr<ID3D12Device> device, std::string name);

public:
	static void LoadShaders();
	static Shader* GetShader(std::string name);
};
