#include "SRVBuffer.hpp"

#include "Graphics.hpp"

using namespace Microsoft::WRL;

SRVBuffer::SRVBuffer(jwstring name, size_t size, unsigned int count) : mName(name), mBufferCount(count) {
	if (count == 0) mBufferCount = Graphics::BufferCount();

	ComPtr<ID3D12Device> device = Graphics::GetDevice();

	mBuffers = new ComPtr<ID3D12Resource>[mBufferCount];
	mMappedBuffers = new UINT8*[mBufferCount];

	for (unsigned int i = 0; i < mBufferCount; i++) {
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mBuffers[i])));

		mBuffers[i]->SetName(name.c_str());

		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(mBuffers[i]->Map(0, &readRange, reinterpret_cast<void**>(&mMappedBuffers[i])));
		ZeroMemory(mMappedBuffers[i], size);
	}
}
SRVBuffer::~SRVBuffer() {
	for (unsigned int i = 0; i < mBufferCount; i++) {
		CD3DX12_RANGE readRange(0, 0);
		mBuffers[i]->Unmap(0, &readRange);
		mBuffers[i].Reset();
	}
	delete[] mMappedBuffers;
	delete[] mBuffers;
}

D3D12_GPU_VIRTUAL_ADDRESS SRVBuffer::GetGPUAddress(unsigned int frameIndex) const {
	return mBuffers[frameIndex]->GetGPUVirtualAddress();
}
