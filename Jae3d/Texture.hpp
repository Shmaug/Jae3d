#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <DirectXMath.h>

#include "jvector.hpp"
#include "Asset.hpp"

class CommandList;
class Shader;

class Texture : public Asset {
public:
	static inline unsigned int ComputeNumMips(unsigned int Width, unsigned int Height) {
		unsigned int HighBit;
		_BitScanReverse((unsigned long*)&HighBit, Width | Height);
		return HighBit + 1;
	}

	JAE_API Texture(jwstring name, int width, int height, int dimension, DXGI_FORMAT format, int mipLevels, void* ddsData, size_t ddsSize);
	JAE_API Texture(jwstring name, MemoryStream &ms);
	JAE_API ~Texture();

	JAE_API void WriteData(MemoryStream &ms);
	JAE_API uint64_t TypeId();

	JAE_API void Upload();

	unsigned int Width() const { return mWidth; }
	unsigned int Height() const { return mHeight; }
	unsigned int Dimension() const { return mDimension; }
	unsigned int MipLevels() const { return mMipLevels; }
	DXGI_FORMAT Format() const { return mFormat; }

	_WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const { return msrvHeap; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor() const { return msrvHeap->GetGPUDescriptorHandleForHeapStart(); }
	_WRL::ComPtr<ID3D12Resource> GetTexture() const { return mTexture; }

private:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mDimension;
	unsigned int mMipLevels;
	DXGI_FORMAT mFormat;

	size_t mDDSDataSize;
	char* mDDSData;

	jvector<D3D12_SUBRESOURCE_DATA> mSubresources;
	_WRL::ComPtr<ID3D12Resource> mTexture;
	_WRL::ComPtr<ID3D12DescriptorHeap> msrvHeap;
};