#pragma once
#include <d3d12.h>
#include <cstdint>
#include <wrl.h>

using namespace Microsoft::WRL;

class CommandQueue {
public:
	CommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type);
	~CommandQueue();

	uint64_t Signal();
	void WaitForFenceValue(uint64_t value);
	bool IsFenceComplete(uint64_t value);
	void Flush();

	ComPtr<ID3D12CommandQueue> GetCommandQueue() const;

private:
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device);

};

