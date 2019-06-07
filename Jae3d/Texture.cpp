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

Texture::Texture(const jwstring& name, unsigned int width, unsigned int height, DXGI_FORMAT format, unsigned int mipLevels)
	: Asset(name), mWidth(width), mHeight(height), mDepth(1), mDimension(D3D12_RESOURCE_DIMENSION_TEXTURE2D), mArraySize(1),
	mFormat(format), mMipLevels(mipLevels), mData(nullptr), mDataSize(0), mIsDDS(false) {}

Texture::Texture(const jwstring& name, unsigned int width, unsigned int height, unsigned int depth, DXGI_FORMAT format, unsigned int mipLevels)
	: Asset(name), mWidth(width), mHeight(height), mDepth(depth), mDimension(D3D12_RESOURCE_DIMENSION_TEXTURE3D), mArraySize(1),
	mFormat(format), mMipLevels(mipLevels), mData(nullptr), mDataSize(0), mIsDDS(false) {}

Texture::Texture(const jwstring& name, unsigned int width, unsigned int height, unsigned int depth,
	D3D12_RESOURCE_DIMENSION dimension, unsigned int arraySize,
	DXGI_FORMAT format, unsigned int mipLevels, const void* data, size_t dataSize, bool isDDS)
	: Asset(name), mWidth(width), mHeight(height), mDepth(depth), mDimension(dimension), mArraySize(arraySize),
	mFormat(format), mMipLevels(mipLevels), mDataSize(dataSize), mIsDDS(isDDS) {

	if (data) {
		mData = new uint8_t[mDataSize];
		memcpy(mData, data, mDataSize);
	}
}

Texture::Texture(const jwstring& name, MemoryStream &ms) : Asset(name, ms) {
	mWidth = ms.Read<uint32_t>();
	mHeight = ms.Read<uint32_t>();
	mDepth = ms.Read<uint32_t>();
	mDimension = (D3D12_RESOURCE_DIMENSION)ms.Read<uint32_t>();
	mArraySize = ms.Read<uint32_t>();
	mMipLevels = ms.Read<uint32_t>();
	mFormat = (DXGI_FORMAT)ms.Read<uint32_t>();

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

uint64_t Texture::TypeId() { return (uint64_t)ASSET_TYPE_TEXTURE; }
void Texture::WriteData(MemoryStream &ms) {
	ms.Write((uint32_t)mWidth);
	ms.Write((uint32_t)mHeight);
	ms.Write((uint32_t)mDepth);
	ms.Write((uint32_t)mDimension);
	ms.Write((uint32_t)mArraySize);
	ms.Write((uint32_t)mMipLevels);
	ms.Write((uint32_t)mFormat);
	ms.Write((uint8_t)(mIsDDS ? 1 : 0));

	ms.Write((uint64_t)mDataSize);
	if (mDataSize > 0)
		ms.Write(reinterpret_cast<const char*>(mData), mDataSize);
}

void Texture::SetPixelData(const void* data) {
	if (data)
		memcpy(mData, data, mDataSize);
	else
		ZeroMemory(mData, mDataSize);
	mIsDDS = false;
}

void Texture::Upload(D3D12_RESOURCE_FLAGS flags, bool makeHeaps) {
	auto device = Graphics::GetDevice();
	mTexture.Reset();

	uint8_t* data = mData;

	vector<D3D12_SUBRESOURCE_DATA> subresources;
	if (mIsDDS) {
		if (mDataSize == 0 || !mData) return;

		HRESULT hr = LoadDDSTextureFromMemory(device.Get(), reinterpret_cast<const uint8_t*>(mData), mDataSize, mTexture.ReleaseAndGetAddressOf(), subresources);

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
		desc.Flags = flags;
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

		if (!data || mDataSize == 0) {
			data = new uint8_t[mHeight * stride * desc.DepthOrArraySize];
			ZeroMemory(data, mHeight * stride * desc.DepthOrArraySize);
		}

		D3D12_SUBRESOURCE_DATA subrsrc;
		subrsrc.pData = data;
		subrsrc.RowPitch = stride;
		subrsrc.SlicePitch = stride * mHeight;
		subresources.push_back(subrsrc);
	}

	mTexture->SetName(mName.c_str());

	auto commandQueue = Graphics::GetCommandQueue();
	auto commandList = commandQueue->GetCommandList(0);

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mTexture.Get(), 0, static_cast<UINT>(subresources.size()));

	// Create the GPU upload buffer.
	ComPtr<ID3D12Resource> uploadBuffer;
	ThrowIfFailed(
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuffer)));
	uploadBuffer->SetName(L"Texture Upload Buffer");
	UpdateSubresources(commandList->D3DCommandList().Get(), mTexture.Get(), uploadBuffer.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	commandList->TransitionResource(mTexture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandQueue->WaitForFenceValue(commandQueue->Execute(commandList));

	if (data != mData) delete[] data;

	mHasDescriptorHeaps = makeHeaps;
	if (makeHeaps) {
		D3D12_RESOURCE_DESC desc = mTexture->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = desc.Format;

		switch (mDimension) {
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			srvDesc.Texture1D.MipLevels = mMipLevels;
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			srvDesc.Texture2D.MipLevels = mMipLevels;
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			srvDesc.Texture3D.MipLevels = mMipLevels;
			break;
		}
		srvDesc.Texture3D.MipLevels = mMipLevels;

		D3D12_UAV_DIMENSION uavdim;
		switch (desc.Dimension) {
		default:
		case (D3D12_RESOURCE_DIMENSION_UNKNOWN):
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_UNKNOWN;
			uavdim = D3D12_UAV_DIMENSION_UNKNOWN;
			break;
		case (D3D12_RESOURCE_DIMENSION_TEXTURE1D):
			srvDesc.Texture1D.MipLevels = desc.MipLevels;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			uavdim = D3D12_UAV_DIMENSION_TEXTURE1D;
			break;
		case (D3D12_RESOURCE_DIMENSION_TEXTURE2D):
			if (mArraySize > 1) {
				if (mArraySize == 6) {
					srvDesc.TextureCube.MipLevels = desc.MipLevels;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				} else
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				uavdim = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			} else {
				srvDesc.Texture2D.MipLevels = desc.MipLevels;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				uavdim = D3D12_UAV_DIMENSION_TEXTURE2D;
			}
			break;
		case (D3D12_RESOURCE_DIMENSION_TEXTURE3D):
			srvDesc.Texture3D.MipLevels = desc.MipLevels;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			uavdim = D3D12_UAV_DIMENSION_TEXTURE3D;
			break;
		}

		mSRVHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		device->CreateShaderResourceView(mTexture.Get(), &srvDesc, mSRVHeap->GetCPUDescriptorHandleForHeapStart());
		mSRVHeap->SetName((mName + L" SRV Heap").c_str());

		if (flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
			mUAVHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
			mUAVHeap->SetName((mName + L" UAV Heap").c_str());
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.ViewDimension = uavdim;
			uavDesc.Format = desc.Format;
			switch (mDimension) {
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				uavDesc.Texture3D.WSize = -1;
				break;
			}
			device->CreateUnorderedAccessView(mTexture.Get(), 0, &uavDesc, mUAVHeap->GetCPUDescriptorHandleForHeapStart());
		}
	}
}
