#pragma once

#include "Util.hpp"

#include "../Common/jstring.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <DirectXMath.h>

class ConstantBuffer {
public:
	jstring mName;

	ConstantBuffer();
	ConstantBuffer(int size, jstring name);
	~ConstantBuffer();

	void Write(const void* ptr, size_t size, unsigned int pos = 0);

	void WriteFloat(float v, unsigned int pos);
	void WriteFloat2(DirectX::XMFLOAT2 v, unsigned int pos);
	void WriteFloat3(DirectX::XMFLOAT3 v, unsigned int pos);
	void WriteFloat4(DirectX::XMFLOAT4 v, unsigned int pos);

	void WriteFloat4x4(DirectX::XMFLOAT4X4 v, unsigned int pos);
	void WriteFloat3x3(DirectX::XMFLOAT3X3 v, unsigned int pos);

	void WriteInt(int v, unsigned int pos);
	void WriteInt2(DirectX::XMINT2 v, unsigned int pos);
	void WriteInt3(DirectX::XMINT3 v, unsigned int pos);
	void WriteInt4(DirectX::XMINT4 v, unsigned int pos);

	void WriteUInt(unsigned int v, unsigned int pos);
	void WriteUInt2(DirectX::XMUINT2 v, unsigned int pos);
	void WriteUInt3(DirectX::XMUINT3 v, unsigned int pos);
	void WriteUInt4(DirectX::XMUINT4 v, unsigned int pos);

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return mCBuffer->GetGPUVirtualAddress(); }

	ConstantBuffer& operator =(ConstantBuffer& rhs) = delete;

private:
	char* buffer;
	_WRL::ComPtr<ID3D12Resource> mCBuffer;
	UINT8* mMappedCBuffer;
};

