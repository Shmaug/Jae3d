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


class Graphics {
public:
	float m_fps;

	HWND m_hWnd;
	RECT m_WindowRect;

	uint32_t m_ClientWidth = 1280;
	uint32_t m_ClientHeight = 720;

	bool Graphics::WarpEnabled() const { return m_UseWarp; };
	bool Graphics::IsInitialized() const { return m_Initialized; };
	bool Graphics::IsFullscreen() const { return m_Fullscreen; };
	bool Graphics::TearingSupported() const { return m_TearingSupported; };
	bool Graphics::VSync() const { return m_VSync; };

	void Graphics::SetVSync(bool vsync) { m_VSync = vsync; }
	int Graphics::GetAndResetFPS() {
		int x = m_fpsCounter;
		m_fpsCounter = 0;
		return x;
	}
	void Graphics::SetFullscreen(bool fullscreen);

	void Graphics::Initialize(HWND hWnd, bool warp);
	void Graphics::Destroy();
	void Graphics::Resize(uint32_t width, uint32_t height);
	void Graphics::Flush(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent);
	void Graphics::StartRenderLoop(HANDLE mutex);

private:
	HANDLE m_mutex;
	// Use WARP adapter
	bool m_UseWarp = false;
	// Set to true once the DX12 objects have been initialized.
	bool m_Initialized = false;
	bool m_Fullscreen = false;
	bool m_TearingSupported = false;
	bool m_VSync = true;
	int m_fpsCounter;

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

	void Graphics::Render(ComPtr<ID3D12Resource> backBuffer);
	static unsigned int RenderLoop(void *data);

	void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap);

	uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t &fenceValue);
};

