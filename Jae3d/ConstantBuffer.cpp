#include "ConstantBuffer.hpp"

#include "Graphics.hpp"
#include "IOUtil.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

ConstantBuffer::ConstantBuffer() : mName(L"") {}
ConstantBuffer::ConstantBuffer(int size, jwstring name, unsigned int count) : mName(name) {
	mCBufferCount = count;
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

void ConstantBuffer::Write(const void* ptr, size_t size, unsigned int pos, unsigned int index) {
	if (index == (unsigned int)-1) {
		for (unsigned int i = 0; i < mCBufferCount; i++)
			memcpy(mMappedCBuffers[i] + pos, ptr, size);
	}else
		memcpy(mMappedCBuffers[index] + pos, ptr, size);
}

void ConstantBuffer::WriteFloat(float v, unsigned int pos, unsigned int index) {
	Write(&v, sizeof(float), pos, index);
}
void ConstantBuffer::WriteFloat2(XMFLOAT2 v, unsigned int pos, unsigned int index){
	Write(&v, sizeof(XMFLOAT2), pos, index);
}
void ConstantBuffer::WriteFloat3(XMFLOAT3 v, unsigned int pos, unsigned int index){
	Write(&v, sizeof(XMFLOAT3), pos, index);
}
void ConstantBuffer::WriteFloat4(XMFLOAT4 v, unsigned int pos, unsigned int index){
	Write(&v, sizeof(XMFLOAT4), pos, index);
}
void ConstantBuffer::WriteFloat4x4(XMFLOAT4X4 v, unsigned int pos, unsigned int index) {
	Write(&v, sizeof(XMFLOAT4X4), pos, index);
}
void ConstantBuffer::WriteFloat3x3(XMFLOAT3X3 v, unsigned int pos, unsigned int index) {
	Write(&v, sizeof(XMFLOAT3X3), pos, index);
}

void ConstantBuffer::WriteInt(int v, unsigned int pos, unsigned int index) {
	Write(&v, sizeof(int), pos, index);
}
void ConstantBuffer::WriteInt2(XMINT2 v, unsigned int pos, unsigned int index){
	Write(&v, sizeof(XMINT2), pos, index);
}
void ConstantBuffer::WriteInt3(XMINT3 v, unsigned int pos, unsigned int index){
	Write(&v, sizeof(XMINT3), pos, index);
}
void ConstantBuffer::WriteInt4(XMINT4 v, unsigned int pos, unsigned int index){
	Write(&v, sizeof(XMINT4), pos, index);
}

void ConstantBuffer::WriteUInt(unsigned int v, unsigned int pos, unsigned int index) {
	Write(&v, sizeof(unsigned int), pos, index);
}
void ConstantBuffer::WriteUInt2(XMUINT2 v, unsigned int pos, unsigned int index) {
	Write(&v, sizeof(XMUINT2), pos, index);
}
void ConstantBuffer::WriteUInt3(XMUINT3 v, unsigned int pos, unsigned int index) {
	Write(&v, sizeof(XMUINT3), pos, index);
}
void ConstantBuffer::WriteUInt4(XMUINT4 v, unsigned int pos, unsigned int index) {
	Write(&v, sizeof(XMUINT4), pos, index);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUAddress(unsigned int index) const {
	return mCBuffers[index]->GetGPUVirtualAddress();
}