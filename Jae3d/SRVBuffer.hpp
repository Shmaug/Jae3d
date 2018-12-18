#pragma once

#include "Util.hpp"
#include "jstring.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <DirectXMath.h>

class SRVBuffer {
public:
	jwstring mName;

	JAE_API SRVBuffer(jwstring name, size_t size, unsigned int count = 0);
	JAE_API ~SRVBuffer();

	JAE_API D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(unsigned int frameIndex) const;
	SRVBuffer& operator =(SRVBuffer& rhs) = delete;

private:
	unsigned int mBufferCount;
	_WRL::ComPtr<ID3D12Resource>* mBuffers;
	UINT8** mMappedBuffers;
};

