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

	static inline size_t BitsPerPixel(_In_ DXGI_FORMAT fmt) {
		switch (fmt) {
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 128;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 96;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_Y416:
		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
			return 64;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_AYUV:
		case DXGI_FORMAT_Y410:
		case DXGI_FORMAT_YUY2:
			return 32;

		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			return 24;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_A8P8:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return 16;

		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_420_OPAQUE:
		case DXGI_FORMAT_NV11:
			return 12;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
			return 8;

		case DXGI_FORMAT_R1_UNORM:
			return 1;

		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			return 4;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return 8;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)

		case DXGI_FORMAT_V408:
			return 24;

		case DXGI_FORMAT_P208:
		case DXGI_FORMAT_V208:
			return 16;

#endif // (_WIN32_WINNT >= _WIN32_WINNT_WIN10)

#if defined(_XBOX_ONE) && defined(_TITLE)

		case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
		case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
		case DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
			return 32;

		case DXGI_FORMAT_D16_UNORM_S8_UINT:
		case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
			return 24;

		case DXGI_FORMAT_R4G4_UNORM:
			return 8;

#endif // _XBOX_ONE && _TITLE

		default:
			return 0;
		}
	}

	JAE_API Texture(const jwstring& name, unsigned int width, unsigned int height, DXGI_FORMAT format, unsigned int mipLevels = 1);
	JAE_API Texture(const jwstring& name, unsigned int width, unsigned int height, unsigned int depth, DXGI_FORMAT format, unsigned int mipLevels = 1);
	JAE_API Texture(const jwstring& name,
		unsigned int width, unsigned int height, unsigned int depth,
		D3D12_RESOURCE_DIMENSION dimension, unsigned int arraySize,
		DXGI_FORMAT format, unsigned int mipLevels, const void* data, size_t dataSize, bool isDDS);

	JAE_API Texture(const jwstring& name, MemoryStream &ms);
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