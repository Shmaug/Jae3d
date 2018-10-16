#pragma once

#include <map>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include <vector>

class RootSignature {
public:
	RootSignature();
	RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion);

	virtual ~RootSignature();

	void Destroy();

	_WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_RootSignature; }

	void SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion);

	const D3D12_ROOT_SIGNATURE_DESC1& GetRootSignatureDesc() const { return m_RootSignatureDesc; }

	uint32_t GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const;
	uint32_t GetNumDescriptors(uint32_t rootIndex) const;

private:
	D3D12_ROOT_SIGNATURE_DESC1 m_RootSignatureDesc;
	_WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

	// Need to know the number of descriptors per descriptor table.
	// A maximum of 32 descriptor tables are supported (since a 32-bit
	// mask is used to represent the descriptor tables in the root signature.
	uint32_t m_NumDescriptorsPerTable[32];

	// A bit mask that represents the root parameter indices that are 
	// descriptor tables for Samplers.
	uint32_t m_SamplerTableBitMask;
	// A bit mask that represents the root parameter indices that are 
	// CBV, UAV, and SRV descriptor tables.
	uint32_t m_DescriptorTableBitMask;
};

class Shader {
public:
	std::string name;
	_WRL::ComPtr<ID3DBlob> vertexBlob;
	_WRL::ComPtr<ID3DBlob> pixelBlob;

	RootSignature m_RootSignature;
	_WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
};

class ShaderLibrary {
	static std::map<std::string, Shader*> shaders;

public:
	static void LoadShaders();
	static Shader* GetShader(std::string name);
};
