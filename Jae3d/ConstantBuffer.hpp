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
	JAE_API ConstantBuffer(int size, jwstring name, unsigned int count);
	JAE_API ~ConstantBuffer();

	JAE_API void Write(const void* ptr, size_t size, unsigned int pos, unsigned int index);

	JAE_API void WriteFloat(float v, unsigned int pos, unsigned int index);
	JAE_API void WriteFloat2(DirectX::XMFLOAT2 v, unsigned int pos, unsigned int index);
	JAE_API void WriteFloat3(DirectX::XMFLOAT3 v, unsigned int pos, unsigned int index);
	JAE_API void WriteFloat4(DirectX::XMFLOAT4 v, unsigned int pos, unsigned int index);

	JAE_API void WriteFloat4x4(DirectX::XMFLOAT4X4 v, unsigned int pos, unsigned int index);
	JAE_API void WriteFloat3x3(DirectX::XMFLOAT3X3 v, unsigned int pos, unsigned int index);

	JAE_API void WriteInt(int v, unsigned int pos, unsigned int index);
	JAE_API void WriteInt2(DirectX::XMINT2 v, unsigned int pos, unsigned int index);
	JAE_API void WriteInt3(DirectX::XMINT3 v, unsigned int pos, unsigned int index);
	JAE_API void WriteInt4(DirectX::XMINT4 v, unsigned int pos, unsigned int index);

	JAE_API void WriteUInt(unsigned int v, unsigned int pos, unsigned int index);
	JAE_API void WriteUInt2(DirectX::XMUINT2 v, unsigned int pos, unsigned int index);
	JAE_API void WriteUInt3(DirectX::XMUINT3 v, unsigned int pos, unsigned int index);
	JAE_API void WriteUInt4(DirectX::XMUINT4 v, unsigned int pos, unsigned int index);

	JAE_API D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(unsigned int index) const;

	ConstantBuffer& operator =(ConstantBuffer& rhs) = delete;

private:
	char* buffer;
	unsigned int mCBufferCount;
	_WRL::ComPtr<ID3D12Resource>* mCBuffers;
	UINT8** mMappedCBuffers;
};

