#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>

#include "Asset.hpp"

class RootSignature;

class Shader : public Asset {
public:
	enum ShaderStage {
		Vertex = 1,
		Hull = 2,
		Domain = 3,
		Geometry = 4,
		Pixel = 5,
		Compute = 6
	};

	Shader(std::string name);
	~Shader();

	void Create();
	void SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	_WRL::ComPtr<ID3DBlob> m_Blobs[6];

	RootSignature *m_RootSignature;
	_WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
};