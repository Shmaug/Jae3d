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

	JAE_API Texture(jwstring name, unsigned int width, unsigned int height, DXGI_FORMAT format, unsigned int mipLevels = 1);
	JAE_API Texture(jwstring name, unsigned int width, unsigned int height, unsigned int depth, DXGI_FORMAT format, unsigned int mipLevels = 1);
	JAE_API Texture(jwstring name,
		unsigned int width, unsigned int height, unsigned int depth,
		D3D12_RESOURCE_DIMENSION dimension, unsigned int arraySize,
		DXGI_FORMAT format, unsigned int mipLevels, const void* data, size_t dataSize, bool isDDS);

	JAE_API Texture(jwstring name, MemoryStream &ms);
	JAE_API ~Texture();

	JAE_API void WriteData(MemoryStream &ms) override;
	JAE_API uint64_t TypeId() override;

	// Upload pixel data to the GPU
	// Create SRV (and UAV if flags has D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) in descriptor tables if makeHeaps is true
	JAE_API void Upload(D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, bool makeHeaps = true);

	const void* GetPixelData() const { return mData; };
	JAE_API void SetPixelData(const void* data);

	unsigned int Width() const { return mWidth; }
	unsigned int Height() const { return mHeight; }
	unsigned int Depth() const { return mDepth; }
	unsigned int ArraySize() const { return mArraySize; }
	D3D12_RESOURCE_DIMENSION Dimension() const { return mDimension; }
	unsigned int MipLevels() const { return mMipLevels; }
	DXGI_FORMAT Format() const { return mFormat; }

	bool HasDescriptors() const { return mHasDescriptorHeaps; }

	_WRL::ComPtr<ID3D12DescriptorHeap> GetSRVDescriptorHeap() const { return mSRVHeap; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptor() const { return mSRVHeap->GetGPUDescriptorHandleForHeapStart(); }
	
	_WRL::ComPtr<ID3D12DescriptorHeap> GetUAVDescriptorHeap() const { return mUAVHeap; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUAVGPUDescriptor() const { return mUAVHeap->GetGPUDescriptorHandleForHeapStart(); }

	_WRL::ComPtr<ID3D12Resource> GetTexture() const { return mTexture; }

private:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mDepth;
	unsigned int mArraySize;
	D3D12_RESOURCE_DIMENSION mDimension;
	unsigned int mMipLevels;
	DXGI_FORMAT mFormat;

	size_t mDataSize;
	uint8_t* mData;
	bool mIsDDS;

	_WRL::ComPtr<ID3D12Resource> mTexture;

	bool mHasDescriptorHeaps;
	_WRL::ComPtr<ID3D12DescriptorHeap> mSRVHeap;
	_WRL::ComPtr<ID3D12DescriptorHeap> mUAVHeap;
};