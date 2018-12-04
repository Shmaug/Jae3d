#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <d3d12.h>

#include "../Common/ShaderAsset.hpp"
#include "../Common/MeshAsset.hpp"

class Shader : public ShaderAsset {
public:
	Shader(jstring name);
	Shader(jstring name, MemoryStream &ms);
	~Shader();

	void Upload();

private:
	friend class CommandList;
	bool SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetPSO(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, MeshAsset::SEMANTIC input);
	void SetCompute(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	jmap<MeshAsset::SEMANTIC, _WRL::ComPtr<ID3D12PipelineState>> mStates;
	_WRL::ComPtr<ID3D12PipelineState> mComputePSO;

	_WRL::ComPtr<ID3D12PipelineState> CreatePSO(MeshAsset::SEMANTIC input);
	void CreateComputePSO();

	bool mCreated = false;
	_WRL::ComPtr<ID3D12RootSignature> mRootSignature;
};