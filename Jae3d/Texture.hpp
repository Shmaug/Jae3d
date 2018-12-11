#pragma once

#include "Common.hpp"
#include "Asset.hpp"

class Texture : public Asset {
public:
	static inline unsigned int ComputeNumMips(unsigned int Width, unsigned int Height) {
		unsigned int HighBit;
		_BitScanReverse((unsigned long*)&HighBit, Width | Height);
		return HighBit + 1;
	}

	JAE_API Texture(jwstring name,
		unsigned int width, unsigned int height, unsigned int depth,
		D3D12_RESOURCE_DIMENSION dimension, unsigned int arraySize,
		DXGI_FORMAT format, ALPHA_MODE alphaMode, unsigned int mipLevels, void* data, size_t dataSize, bool isDDS);

	JAE_API Texture(jwstring name, MemoryStream &ms);
	JAE_API ~Texture();

	JAE_API void WriteData(MemoryStream &ms);
	JAE_API uint64_t TypeId();

	JAE_API void Upload();

	unsigned int Width() const { return mWidth; }
	unsigned int Height() const { return mHeight; }
	unsigned int Depth() const { return mDepth; }
	unsigned int ArraySize() const { return mArraySize; }
	D3D12_RESOURCE_DIMENSION Dimension() const { return mDimension; }
	unsigned int MipLevels() const { return mMipLevels; }
	DXGI_FORMAT Format() const { return mFormat; }
	ALPHA_MODE AlphaMode() const { return mAlphaMode; }

	_WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const { return msrvHeap; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor() const { return msrvHeap->GetGPUDescriptorHandleForHeapStart(); }
	_WRL::ComPtr<ID3D12Resource> GetTexture() const { return mTexture; }

private:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mDepth;
	unsigned int mArraySize;
	D3D12_RESOURCE_DIMENSION mDimension;
	unsigned int mMipLevels;
	DXGI_FORMAT mFormat;
	ALPHA_MODE mAlphaMode;

	size_t mDataSize;
	uint8_t* mData;
	bool mIsDDS;

	jvector<D3D12_SUBRESOURCE_DATA> mSubresources;
	_WRL::ComPtr<ID3D12Resource> mTexture;
	_WRL::ComPtr<ID3D12DescriptorHeap> msrvHeap;
};