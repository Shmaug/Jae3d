#pragma once

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
// D3D12 extension library.
#include "d3dx12.h"

#include "Util.h"
#include "Mathf.h"

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

// The number of swap chain back buffers.
const uint8_t g_NumFrames = 2;

class Graphics {
public:
	HWND g_hWnd;
	RECT g_WindowRect;

	uint32_t g_ClientWidth = 1280;
	uint32_t g_ClientHeight = 720;

	// Use WARP adapter
	bool g_UseWarp = false;

	// Set to true once the DX12 objects have been initialized.
	bool g_IsInitialized = false;
	bool g_Fullscreen = false;
	bool g_TearingSupported = false;
	bool g_VSync = false;

	// DirectX 12 Objects
	ComPtr<ID3D12Device2> g_Device;
	ComPtr<ID3D12CommandQueue> g_CommandQueue;
	ComPtr<IDXGISwapChain4> g_SwapChain;
	ComPtr<ID3D12Resource> g_BackBuffers[g_NumFrames];
	ComPtr<ID3D12GraphicsCommandList> g_CommandList;
	ComPtr<ID3D12CommandAllocator> g_CommandAllocators[g_NumFrames];
	ComPtr<ID3D12DescriptorHeap> g_RTVDescriptorHeap;
	UINT g_RTVDescriptorSize;
	UINT g_CurrentBackBufferIndex;

	// Synchronization objects
	ComPtr<ID3D12Fence> g_Fence;
	uint64_t g_FenceValue = 0;
	uint64_t g_FrameFenceValues[g_NumFrames] = {};
	HANDLE g_FenceEvent;

	bool Graphics::CheckTearingSupport();

	ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);

	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);
	ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device);

	HANDLE CreateEventHandle();

	void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration);

	void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap);

	uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t & fenceValue);

	void Graphics::Initialize();
	void Graphics::SetFullscreen(bool fullscreen);
	void Graphics::Resize(uint32_t width, uint32_t height);
	void Graphics::ClearBackBuffer(ComPtr<ID3D12Resource> backBuffer, Color color);
	void Graphics::Present(ComPtr<ID3D12Resource> backBuffer);
	void Graphics::Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent);
};

