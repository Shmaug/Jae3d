#include "ConstantBuffer.hpp"

#include "Graphics.hpp"
#include "IOUtil.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

ConstantBuffer::ConstantBuffer() : mName(L"") {}
ConstantBuffer::ConstantBuffer(size_t size, const jwstring& name, unsigned int count) : mName(name), mCBufferCount(count) {
	if (count == 0) mCBufferCount = Graphics::BufferCount();

	ComPtr<ID3D12Device> device = Graphics::GetDevice();

	size_t bufSize = AlignUp(size, 256);

	mCBuffers = new ComPtr<ID3D12Resource>[mCBufferCount];
	mMappedCBuffers = new UINT8*[mCBufferCount];

	for (unsigned int i = 0; i < mCBufferCount; i++) {
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mCBuffers[i])));

		mCBuffers[i]->SetName(name.c_str());

		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(mCBuffers[i]->Map(0, &readRange, reinterpret_cast<void**>(&mMappedCBuffers[i])));
		ZeroMemory(mMappedCBuffers[i], bufSize);
	}
}
ConstantBuffer::~ConstantBuffer() {
	for (unsigned int i = 0; i < mCBufferCount; i++) {
		CD3DX12_RANGE readRange(0, 0);
		mCBuffers[i]->Unmap(0, &readRange);
		mCBuffers[i].Reset();
	}
	delete[] mMappedCBuffers;
	delete[] mCBuffers;
}

void ConstantBuffer::Write(const void* ptr, size_t size, unsigned int pos, unsigned int frameIndex) {
	if (frameIndex == (unsigned int)-1)
		for (unsigned int i = 0; i < mCBufferCount; i++)
			memcpy(mMappedCBuffers[i] + pos, ptr, size);
	else
		memcpy(mMappedCBuffers[frameIndex] + pos, ptr, size);
}

void ConstantBuffer::WriteFloat (const float &v, unsigned int pos, unsigned int frameIndex) {
	Write(&v, sizeof(float), pos, frameIndex);
}
void ConstantBuffer::WriteFloat2(const XMFLOAT2 &v, unsigned int pos, unsigned int frameIndex){
	Write(&v, sizeof(XMFLOAT2), pos, frameIndex);
}
void ConstantBuffer::WriteFloat3(const XMFLOAT3 &v, unsigned int pos, unsigned int frameIndex){
	Write(&v, sizeof(XMFLOAT3), pos, frameIndex);
}
void ConstantBuffer::WriteFloat4(const XMFLOAT4 &v, unsigned int pos, unsigned int frameIndex){
	Write(&v, sizeof(XMFLOAT4), pos, frameIndex);
}
void ConstantBuffer::WriteFloat4x4(const XMFLOAT4X4 &v, unsigned int pos, unsigned int frameIndex) {
	Write(&v, sizeof(XMFLOAT4X4), pos, frameIndex);
}
void ConstantBuffer::WriteFloat3x3(const XMFLOAT3X3 &v, unsigned int pos, unsigned int frameIndex) {
	Write(&v, sizeof(XMFLOAT3X3), pos, frameIndex);
}

void ConstantBuffer::WriteInt (const int &v, unsigned int pos, unsigned int frameIndex) {
	Write(&v, sizeof(int), pos, frameIndex);
}
void ConstantBuffer::WriteInt2(const XMINT2 &v, unsigned int pos, unsigned int frameIndex){
	Write(&v, sizeof(XMINT2), pos, frameIndex);
}
void ConstantBuffer::WriteInt3(const XMINT3 &v, unsigned int pos, unsigned int frameIndex){
	Write(&v, sizeof(XMINT3), pos, frameIndex);
}
void ConstantBuffer::WriteInt4(const XMINT4 &v, unsigned int pos, unsigned int frameIndex){
	Write(&v, sizeof(XMINT4), pos, frameIndex);
}

void ConstantBuffer::WriteUInt (const unsigned int &v, unsigned int pos, unsigned int frameIndex) {
	Write(&v, sizeof(unsigned int), pos, frameIndex);
}
void ConstantBuffer::WriteUInt2(const XMUINT2 &v, unsigned int pos, unsigned int frameIndex) {
	Write(&v, sizeof(XMUINT2), pos, frameIndex);
}
void ConstantBuffer::WriteUInt3(const XMUINT3 &v, unsigned int pos, unsigned int frameIndex) {
	Write(&v, sizeof(XMUINT3), pos, frameIndex);
}
void ConstantBuffer::WriteUInt4(const XMUINT4 &v, unsigned int pos, unsigned int frameIndex) {
	Write(&v, sizeof(XMUINT4), pos, frameIndex);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUAddress(unsigned int frameIndex) const {
	return mCBuffers[frameIndex]->GetGPUVirtualAddress();
}