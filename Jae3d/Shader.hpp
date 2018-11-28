#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include <map>

#include "../Common/ShaderAsset.hpp"
#include "../Common/MeshAsset.hpp"

class RootSignature;

class Shader : public ShaderAsset {
public:
	Shader(std::string name);
	Shader(std::string name, MemoryStream &ms);
	~Shader();

	void Upload();
	void SetPSO(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, MeshAsset::SEMANTIC input);

private:
	friend class CommandList;
	bool SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	std::map<MeshAsset::SEMANTIC, _WRL::ComPtr<ID3D12PipelineState>> m_States;

	_WRL::ComPtr<ID3D12PipelineState> CreatePSO(MeshAsset::SEMANTIC input);

	bool m_Created = false;
	_WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
};