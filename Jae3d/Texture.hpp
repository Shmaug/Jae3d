#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include <DirectXMath.h>
#include "d3dx12.hpp"

#include "../Common/TextureAsset.hpp"
#include "../Common/TextureAsset.hpp"

class CommandList;

class Texture : public TextureAsset {
public:
	Texture(std::string name, int width, int height, int dimension, DXGI_FORMAT format, int mipLevels=0);
	Texture(std::string name, MemoryStream &ms);
	~Texture();

	void Create();

	_WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const { return m_srvHeap; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor() const { return m_srvHeap->GetGPUDescriptorHandleForHeapStart(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_Texture->GetGPUVirtualAddress(); }

private:
	_WRL::ComPtr<ID3D12Resource> m_Texture;
	_WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
};

