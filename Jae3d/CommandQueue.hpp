#pragma once

#include "Util.hpp"

#include <wrl.h>
#include <d3d12.h>

#include <memory>
#include <cstdint>
#include <queue>

class CommandList;

class JAE_API CommandQueue {
public:
	CommandQueue(_WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	~CommandQueue();

	uint64_t Signal();
	void WaitForFenceValue(uint64_t value);
	bool IsFenceComplete(uint64_t value);
	void Flush();

	_WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const;
	std::shared_ptr<CommandList> GetCommandList(unsigned int frameIndex);

	uint64_t Execute(std::shared_ptr<CommandList> commandList);

private:
	_WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	std::shared_ptr<CommandList> CreateCommandList(_WRL::ComPtr<ID3D12CommandAllocator> allocator);
	
	struct CommandAllocatorEntry {
		uint64_t fenceValue;
		_WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	D3D12_COMMAND_LIST_TYPE mCommandListType;
	_WRL::ComPtr<ID3D12Device2> md3d12Device;
	_WRL::ComPtr<ID3D12CommandQueue> md3d12CommandQueue;
	_WRL::ComPtr<ID3D12Fence> md3d12Fence;
	HANDLE mFenceEvent;
	uint64_t mFenceValue;

	std::queue<CommandAllocatorEntry> mCommandAllocatorQueue;
	std::queue<std::shared_ptr<CommandList>> mCommandListQueue;
};

