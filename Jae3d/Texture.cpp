#include "Texture.hpp"

#include <comdef.h>
#include <vector>

#include "Graphics.hpp"
#include "CommandQueue.hpp"
#include "CommandList.hpp"
#include "MemoryStream.hpp"
#include "AssetFile.hpp"

#include "dds/DDSTextureLoader.hpp"

#include "Shader.hpp"

using namespace Microsoft::WRL;
using namespace std;
using namespace DirectX;

inline size_t BitsPerPixel(_In_ DXGI_FORMAT fmt) {
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

Texture::Texture(jwstring name, unsigned int width, unsigned int height, unsigned int depth,
	D3D12_RESOURCE_DIMENSION dimension, unsigned int arraySize,
	DXGI_FORMAT format, ALPHA_MODE alphaMode, unsigned int mipLevels, void* data, size_t dataSize, bool isDDS)
	: Asset(name), mWidth(width), mHeight(height), mDepth(depth), mDimension(dimension), mArraySize(arraySize), mFormat(format), mAlphaMode(alphaMode), mMipLevels(mipLevels), mDataSize(dataSize), mIsDDS(isDDS) {
	mData = new uint8_t[mDataSize];
	memcpy(mData, data, dataSize);
}

Texture::Texture(jwstring name, MemoryStream &ms) : Asset(name, ms) {
	mWidth = ms.Read<uint32_t>();
	mHeight = ms.Read<uint32_t>();
	mDepth = ms.Read<uint32_t>();
	mDimension = (D3D12_RESOURCE_DIMENSION)ms.Read<uint32_t>();
	mArraySize = ms.Read<uint32_t>();
	mMipLevels = ms.Read<uint32_t>();
	mFormat = (DXGI_FORMAT)ms.Read<uint32_t>();
	mAlphaMode = (ALPHA_MODE)ms.Read<uint32_t>();

	mIsDDS = ms.Read<uint8_t>();
	mDataSize = ms.Read<uint64_t>();
	if (mDataSize > 0) {
		mData = new uint8_t[mDataSize];
		ms.Read(reinterpret_cast<char*>(mData), mDataSize);
	} else
		mData = nullptr;
}

Texture::~Texture() {
	if (mData) delete[] mData;
}

uint64_t Texture::TypeId() { return (uint64_t)AssetFile::TYPEID_TEXTURE; }

void Texture::WriteData(MemoryStream &ms) {
	ms.Write((uint32_t)mWidth);
	ms.Write((uint32_t)mHeight);
	ms.Write((uint32_t)mDepth);
	ms.Write((uint32_t)mDimension);
	ms.Write((uint32_t)mArraySize);
	ms.Write((uint32_t)mMipLevels);
	ms.Write((uint32_t)mFormat);
	ms.Write((uint32_t)mAlphaMode);
	ms.Write((uint8_t)(mIsDDS ? 1 : 0));

	ms.Write((uint64_t)mDataSize);
	if (mDataSize > 0)
		ms.Write(reinterpret_cast<const char*>(mData), mDataSize);
}

void Texture::Upload() {
	if (mDataSize == 0 || !mData) return;
	auto device = Graphics::GetDevice();

	vector<D3D12_SUBRESOURCE_DATA> subresources;
	if (mIsDDS) {
		HRESULT hr = FAILED(LoadDDSTextureFromMemory(device.Get(), reinterpret_cast<const uint8_t*>(mData), mDataSize, mTexture.ReleaseAndGetAddressOf(), subresources));

		if (FAILED(hr)) {
			_com_error e(hr);
			OutputDebugf(L"Failed to load dds texture %s: %s\n", mName.c_str(), e.ErrorMessage());
			throw exception();
		}
	} else {
		HRESULT hr = E_FAIL;

		D3D12_RESOURCE_DESC desc = {};
		desc.Width = mWidth;
		desc.Height = mHeight;
		desc.MipLevels = static_cast<UINT16>(mMipLevels);
		desc.DepthOrArraySize = (mDimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? static_cast<UINT16>(mDepth) : static_cast<UINT16>(mArraySize);
		desc.Format = mFormat;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Dimension = mDimension;

		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

		ThrowIfFailed(device->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&mTexture)));

		size_t stride = mWidth * (BitsPerPixel(mFormat) / 8);

		D3D12_SUBRESOURCE_DATA subrsrc;
		subrsrc.pData = mData;
		subrsrc.RowPitch = stride;
		subrsrc.SlicePitch = stride * mHeight;
		subresources.push_back(subrsrc);
	}

	auto commandQueue = Graphics::GetCommandQueue();
	auto commandList = commandQueue->GetCommandList(0);

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mTexture.Get(), 0, static_cast<UINT>(subresources.size()));

	// Create the GPU upload buffer.
	ComPtr<ID3D12Resource> uploadHeap;
	ThrowIfFailed(
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadHeap)));

	UpdateSubresources(commandList->D3DCommandList().Get(), mTexture.Get(), uploadHeap.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	commandList->TransitionResource(mTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandQueue->WaitForFenceValue(commandQueue->Execute(commandList));

	D3D12_RESOURCE_DESC desc = mTexture->GetDesc();

	D3D12_SRV_DIMENSION dim;
	switch (desc.Dimension) {
	default:
	case (D3D12_RESOURCE_DIMENSION_UNKNOWN):
		dim =  D3D12_SRV_DIMENSION_UNKNOWN;
		break;
	case (D3D12_RESOURCE_DIMENSION_BUFFER):
		dim =  D3D12_SRV_DIMENSION_BUFFER;
		break;
	case (D3D12_RESOURCE_DIMENSION_TEXTURE1D):
		dim =  D3D12_SRV_DIMENSION_TEXTURE1D;
		break;
	case (D3D12_RESOURCE_DIMENSION_TEXTURE2D):
		dim =  D3D12_SRV_DIMENSION_TEXTURE2D;
		break;
	case (D3D12_RESOURCE_DIMENSION_TEXTURE3D):
		dim =  D3D12_SRV_DIMENSION_TEXTURE3D;
		break;
	}

	msrvHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	
	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = dim;
	srvDesc.Format = desc.Format;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	device->CreateShaderResourceView(mTexture.Get(), &srvDesc, msrvHeap->GetCPUDescriptorHandleForHeapStart());
}
