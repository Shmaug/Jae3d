#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>

#include "../Common/ShaderAsset.hpp"

class RootSignature;

class Shader : public ShaderAsset {
public:
	Shader(std::string name);
	Shader(std::string name, MemoryStream &ms);
	~Shader();

	void Create();
	bool SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	bool m_Created = false;
	RootSignature *m_RootSignature;
	_WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
};