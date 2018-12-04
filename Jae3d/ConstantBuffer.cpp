#include "ConstantBuffer.hpp"

#include "Graphics.hpp"
#include "../Common/IOUtil.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

ConstantBuffer::ConstantBuffer() : mName("") {}
ConstantBuffer::ConstantBuffer(int size, jstring name) : mName(name) {
	ComPtr<ID3D12Device> device = Graphics::GetDevice();

	size_t bufSize = AlignUp(size, 256);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mCBuffer)));

	mCBuffer->SetName(utf8toUtf16(name).c_str());

	CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(mCBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mMappedCBuffer)));
	ZeroMemory(mMappedCBuffer, bufSize);
}
ConstantBuffer::~ConstantBuffer() {
	CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	mCBuffer->Unmap(0, &readRange);
	mCBuffer.Reset();
}

void ConstantBuffer::Write(const void* ptr, size_t size, unsigned int pos) {
	memcpy(mMappedCBuffer + pos, ptr, size);
}

void ConstantBuffer::WriteFloat(float v, unsigned int pos) {
	Write(&v, sizeof(float), pos);
}
void ConstantBuffer::WriteFloat2(XMFLOAT2 v, unsigned int pos){
	Write(&v, sizeof(XMFLOAT2), pos);
}
void ConstantBuffer::WriteFloat3(XMFLOAT3 v, unsigned int pos){
	Write(&v, sizeof(XMFLOAT3), pos);
}
void ConstantBuffer::WriteFloat4(XMFLOAT4 v, unsigned int pos){
	Write(&v, sizeof(XMFLOAT4), pos);
}
void ConstantBuffer::WriteFloat4x4(XMFLOAT4X4 v, unsigned int pos) {
	Write(&v, sizeof(XMFLOAT4X4), pos);
}
void ConstantBuffer::WriteFloat3x3(XMFLOAT3X3 v, unsigned int pos) {
	Write(&v, sizeof(XMFLOAT3X3), pos);
}

void ConstantBuffer::WriteInt(int v, unsigned int pos) {
	Write(&v, sizeof(int), pos);
}
void ConstantBuffer::WriteInt2(XMINT2 v, unsigned int pos){
	Write(&v, sizeof(XMINT2), pos);
}
void ConstantBuffer::WriteInt3(XMINT3 v, unsigned int pos){
	Write(&v, sizeof(XMINT3), pos);
}
void ConstantBuffer::WriteInt4(XMINT4 v, unsigned int pos){
	Write(&v, sizeof(XMINT4), pos);
}

void ConstantBuffer::WriteUInt(unsigned int v, unsigned int pos) {
	Write(&v, sizeof(unsigned int), pos);
}
void ConstantBuffer::WriteUInt2(XMUINT2 v, unsigned int pos) {
	Write(&v, sizeof(XMUINT2), pos);
}
void ConstantBuffer::WriteUInt3(XMUINT3 v, unsigned int pos) {
	Write(&v, sizeof(XMUINT3), pos);
}
void ConstantBuffer::WriteUInt4(XMUINT4 v, unsigned int pos) {
	Write(&v, sizeof(XMUINT4), pos);
}