#pragma once

#include "Util.hpp"

#include "jstring.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <DirectXMath.h>

class ConstantBuffer {
public:
	jwstring mName;

	JAE_API ConstantBuffer();
	JAE_API ConstantBuffer(size_t size, const jwstring& name, unsigned int count);
	JAE_API ~ConstantBuffer();

	JAE_API void Write(const void* ptr, size_t size, unsigned int pos, unsigned int frameIndex);

	JAE_API void WriteFloat (const float &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteFloat2(const DirectX::XMFLOAT2 &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteFloat3(const DirectX::XMFLOAT3 &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteFloat4(const DirectX::XMFLOAT4 &v, unsigned int pos, unsigned int frameIndex);

	JAE_API void WriteFloat4x4(const DirectX::XMFLOAT4X4 &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteFloat3x3(const DirectX::XMFLOAT3X3 &v, unsigned int pos, unsigned int frameIndex);

	JAE_API void WriteInt (const int &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteInt2(const DirectX::XMINT2 &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteInt3(const DirectX::XMINT3 &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteInt4(const DirectX::XMINT4 &v, unsigned int pos, unsigned int frameIndex);

	JAE_API void WriteUInt (const unsigned int &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteUInt2(const DirectX::XMUINT2 &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteUInt3(const DirectX::XMUINT3 &v, unsigned int pos, unsigned int frameIndex);
	JAE_API void WriteUInt4(const DirectX::XMUINT4 &v, unsigned int pos, unsigned int frameIndex);

	JAE_API bool Equals(const ConstantBuffer& cb, unsigned int frameIndex) const;

	JAE_API D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(unsigned int frameIndex) const;

	ConstantBuffer& operator =(ConstantBuffer& rhs) = delete;

private:
	unsigned int mCBufferCount;
	size_t mSize;
	size_t mMappedSize;
	_WRL::ComPtr<ID3D12Resource>* mCBuffers;
	UINT8** mMappedCBuffers;
};

