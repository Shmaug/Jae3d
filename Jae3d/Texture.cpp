#include "Texture.hpp"

#include <comdef.h>
#include <vector>

#include "Graphics.hpp"
#include "CommandQueue.hpp"
#include "CommandList.hpp"

#include "../Common/dds/DDSTextureLoader.hpp"

#include "Shader.hpp"

using namespace Microsoft::WRL;
using namespace std;
using namespace DirectX;

Texture::Texture(jstring name, int width, int height, int dimension, DXGI_FORMAT format, int mipLevels, void* ddsData, size_t ddsSize)
	: TextureAsset(name, width, height, dimension, format, mipLevels, ddsData, ddsSize) {}
Texture::Texture(jstring name, MemoryStream &ms) : TextureAsset(name, ms) {}

Texture::~Texture() {}

void Texture::Upload() {
	auto device = Graphics::GetDevice();

	vector<D3D12_SUBRESOURCE_DATA> subresources;
	HRESULT hr = FAILED(LoadDDSTextureFromMemory(device.Get(), reinterpret_cast<const uint8_t*>(mDDSData), mDDSDataSize, mTexture.ReleaseAndGetAddressOf(), subresources));

	if (FAILED(hr)){
		_com_error e(hr);
		OutputDebugf("Failed to load dds texture %s: %s\n", mName.c_str(), e.ErrorMessage());
		throw exception();
	}

	auto commandQueue = Graphics::GetCommandQueue();
	auto commandList = commandQueue->GetCommandList();

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mTexture.Get(), 0,
		static_cast<UINT>(subresources.size()));

	// Create the GPU upload buffer.
	ComPtr<ID3D12Resource> uploadHeap;
	ThrowIfFailed(
		device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadHeap.GetAddressOf())));

	UpdateSubresources(commandList->D3DCommandList().Get(), mTexture.Get(), uploadHeap.Get(),
		0, 0, static_cast<UINT>(subresources.size()), subresources.data());

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
