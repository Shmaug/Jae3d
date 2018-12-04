#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <DirectXMath.h>

#include "../Common/jvector.hpp"
#include "../Common/TextureAsset.hpp"

class CommandList;
class Shader;

class Texture : public TextureAsset {
public:
	Texture(jstring name, int width, int height, int dimension, DXGI_FORMAT format, int mipLevels, void* ddsData, size_t ddsSize);
	Texture(jstring name, MemoryStream &ms);
	~Texture();

	void Upload();

	_WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const { return msrvHeap; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor() const { return msrvHeap->GetGPUDescriptorHandleForHeapStart(); }
	_WRL::ComPtr<ID3D12Resource> GetTexture() const { return mTexture; }

private:
	jvector<D3D12_SUBRESOURCE_DATA> mSubresources;
	_WRL::ComPtr<ID3D12Resource> mTexture;
	_WRL::ComPtr<ID3D12DescriptorHeap> msrvHeap;
};

