#pragma once
#include <d3d12.h>
#include <cstdint>
#include <wrl.h>
#include <queue>

class CommandQueue {
public:
	CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	~CommandQueue();

	uint64_t Signal();
	void WaitForFenceValue(uint64_t value);
	bool IsFenceComplete(uint64_t value);
	void Flush();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

	uint64_t Execute(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);
	
	struct CommandAllocatorEntry {
		uint64_t fenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	D3D12_COMMAND_LIST_TYPE m_CommandListType;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_d3d12Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_d3d12Fence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceValue;

	std::queue<CommandAllocatorEntry> m_CommandAllocatorQueue;
	std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>> m_CommandListQueue;
};

