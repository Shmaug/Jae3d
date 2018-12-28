#pragma once

#include <wrl.h>
#include "Common.hpp"

class DescriptorTable {
public:
	JAE_API DescriptorTable(unsigned int size);
	JAE_API ~DescriptorTable();

	void SetSRV(unsigned int i, std::shared_ptr<Texture> &value) { SetTexture(i, value, false); }
	void SetUAV(unsigned int i, std::shared_ptr<Texture> &value) { SetTexture(i, value, true); }

	_WRL::ComPtr<ID3D12DescriptorHeap> D3DHeap() const { return mHeap; }
	D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor() const { return mHeap->GetCPUDescriptorHandleForHeapStart(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GpuDescriptor() const { return mHeap->GetGPUDescriptorHandleForHeapStart(); }

private:
	_WRL::ComPtr<ID3D12DescriptorHeap> mHeap;
	unsigned int mSize;

	JAE_API void SetTexture(unsigned int i, std::shared_ptr<Texture> tex, bool uav);
};

