#include "CommandQueue.hpp"
#include <assert.h>

#include "CommandList.hpp"

using namespace std;
using namespace Microsoft::WRL;

CommandQueue::CommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type) : mFenceValue(0), mCommandListType(type), md3d12Device(device) {
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(md3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&md3d12CommandQueue)));
	ThrowIfFailed(md3d12Device->CreateFence(mFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&md3d12Fence)));

	mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(mFenceEvent && "Failed to create fence event handle.");
}
CommandQueue::~CommandQueue() {
	::CloseHandle(mFenceEvent);
}

uint64_t CommandQueue::Signal() {
	uint64_t fenceValue = ++mFenceValue;
	md3d12CommandQueue->Signal(md3d12Fence.Get(), fenceValue);
	return fenceValue;
}
void CommandQueue::WaitForFenceValue(uint64_t value) {
	if (!IsFenceComplete(value)) {
		md3d12Fence->SetEventOnCompletion(value, mFenceEvent);
		::WaitForSingleObject(mFenceEvent, DWORD_MAX);
	}
}
bool CommandQueue::IsFenceComplete(uint64_t value) {
	return md3d12Fence->GetCompletedValue() >= value;
}
void CommandQueue::Flush() {
	WaitForFenceValue(Signal());
}

ComPtr<ID3D12CommandQueue> CommandQueue::GetCommandQueue() const {
	return md3d12CommandQueue;
}
shared_ptr<CommandList> CommandQueue::GetCommandList() {
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	shared_ptr<CommandList> commandList;

	if (!mCommandAllocatorQueue.empty() && IsFenceComplete(mCommandAllocatorQueue.front().fenceValue)) {
		commandAllocator = mCommandAllocatorQueue.front().commandAllocator;
		mCommandAllocatorQueue.pop();
		ThrowIfFailed(commandAllocator->Reset());
	} else
		commandAllocator = CreateCommandAllocator();

	if (!mCommandListQueue.empty()) {
		commandList = mCommandListQueue.front();
		mCommandListQueue.pop();
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
	ThrowIfFailed(md3d12Device->CreateCommandAllocator(mCommandListType, IID_PPV_ARGS(&commandAllocator)));
	return commandAllocator;
}
shared_ptr<CommandList> CommandQueue::CreateCommandList(ComPtr<ID3D12CommandAllocator> allocator) {
	return make_shared<CommandList>(md3d12Device, mCommandListType, allocator);
}

uint64_t CommandQueue::Execute(shared_ptr<CommandList> commandList) {
	commandList->D3DCommandList()->Close();

	ID3D12CommandAllocator* commandAllocator;
	UINT dataSize = sizeof(commandAllocator);
	ThrowIfFailed(commandList->D3DCommandList()->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

	ID3D12CommandList* const ppCommandLists[] = { commandList->D3DCommandList().Get() };

	md3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
	uint64_t fenceValue = Signal();

	mCommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
	mCommandListQueue.push(commandList);

	commandAllocator->Release();

	return fenceValue;
}