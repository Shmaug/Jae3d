#include "CommandQueue.hpp"
#include "Util.hpp"
#include <assert.h>

#include "CommandList.hpp"

using namespace std;
using namespace Microsoft::WRL;

CommandQueue::CommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type) : m_FenceValue(0), m_CommandListType(type), m_d3d12Device(device) {
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(m_d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
	ThrowIfFailed(m_d3d12Device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));

	m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(m_FenceEvent && "Failed to create fence event handle.");
}
CommandQueue::~CommandQueue() {
	::CloseHandle(m_FenceEvent);
}

uint64_t CommandQueue::Signal() {
	uint64_t fenceValue = ++m_FenceValue;
	m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), fenceValue);
	return fenceValue;
}
void CommandQueue::WaitForFenceValue(uint64_t value) {
	if (!IsFenceComplete(value)) {
		m_d3d12Fence->SetEventOnCompletion(value, m_FenceEvent);
		::WaitForSingleObject(m_FenceEvent, DWORD_MAX);
	}
}
bool CommandQueue::IsFenceComplete(uint64_t value) {
	return m_d3d12Fence->GetCompletedValue() >= value;
}
void CommandQueue::Flush() {
	WaitForFenceValue(Signal());
}

ComPtr<ID3D12CommandQueue> CommandQueue::GetCommandQueue() const {
	return m_d3d12CommandQueue;
}
shared_ptr<CommandList> CommandQueue::GetCommandList() {
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	shared_ptr<CommandList> commandList;

	if (!m_CommandAllocatorQueue.empty() && IsFenceComplete(m_CommandAllocatorQueue.front().fenceValue)) {
		commandAllocator = m_CommandAllocatorQueue.front().commandAllocator;
		m_CommandAllocatorQueue.pop();
		ThrowIfFailed(commandAllocator->Reset());
	} else
		commandAllocator = CreateCommandAllocator();

	if (!m_CommandListQueue.empty()) {
		commandList = m_CommandListQueue.front();
		m_CommandListQueue.pop();
		commandList->Reset(commandAllocator);
	} else
		commandList = CreateCommandList(commandAllocator);

	// Associate the command allocator with the command list so that it can be
	// retrieved when the command list is executed.
	ThrowIfFailed(commandList->D3DCommandList()->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

	return commandList;
}

// D3D12 Creation
ComPtr<ID3D12CommandAllocator> CommandQueue::CreateCommandAllocator() {
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ThrowIfFailed(m_d3d12Device->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&commandAllocator)));
	return commandAllocator;
}
shared_ptr<CommandList> CommandQueue::CreateCommandList(ComPtr<ID3D12CommandAllocator> allocator) {
	return make_shared<CommandList>(m_d3d12Device, m_CommandListType, allocator);
}

uint64_t CommandQueue::Execute(shared_ptr<CommandList> commandList) {
	commandList->D3DCommandList()->Close();

	ID3D12CommandAllocator* commandAllocator;
	UINT dataSize = sizeof(commandAllocator);
	ThrowIfFailed(commandList->D3DCommandList()->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

	ID3D12CommandList* const ppCommandLists[] = { commandList->D3DCommandList().Get() };

	m_d3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
	uint64_t fenceValue = Signal();

	m_CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
	m_CommandListQueue.push(commandList);

	commandAllocator->Release();

	return fenceValue;
}