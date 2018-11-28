#pragma once

#include <memory>

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include <cstdint>
#include <queue>

class CommandList;

class CommandQueue {
public:
	CommandQueue(_WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	~CommandQueue();

	uint64_t Signal();
	void WaitForFenceValue(uint64_t value);
	bool IsFenceComplete(uint64_t value);
	void Flush();

	_WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const;
	std::shared_ptr<CommandList> GetCommandList();

	uint64_t Execute(std::shared_ptr<CommandList> commandList);

private:
	_WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	std::shared_ptr<CommandList> CreateCommandList(_WRL::ComPtr<ID3D12CommandAllocator> allocator);
	
	struct CommandAllocatorEntry {
		uint64_t fenceValue;
		_WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	D3D12_COMMAND_LIST_TYPE m_CommandListType;
	_WRL::ComPtr<ID3D12Device2> m_d3d12Device;
	_WRL::ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
	_WRL::ComPtr<ID3D12Fence> m_d3d12Fence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceValue;

	std::queue<CommandAllocatorEntry> m_CommandAllocatorQueue;
	std::queue<std::shared_ptr<CommandList>> m_CommandListQueue;
};

