#pragma once

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <memory>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
// D3D12 extension library.
#include "d3dx12.h"

#include "CommandQueue.h"

class Game;

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

	std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;
	bool WarpEnabled() const { return m_UseWarp; };
	bool IsInitialized() const { return m_Initialized; };
	bool IsFullscreen() const { return m_Fullscreen; };
	bool TearingSupported() const { return m_TearingSupported; };
	bool VSync() const { return m_VSync; };

	void SetVSync(bool vsync) { m_VSync = vsync; }
	int GetAndResetFPS() {
		int x = m_fpsCounter;
		m_fpsCounter = 0;
		return x;
	}
	void SetFullscreen(bool fullscreen);

	void Initialize(HWND hWnd, bool warp, Game* game);
	void Destroy();
	void Resize(uint32_t width, uint32_t height);
	void StartRenderLoop(HANDLE mutex);
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView();

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
	Game* m_game;

	bool CheckTearingSupport();
	Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);

	Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
	Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, uint32_t width, uint32_t height);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
	
	std::shared_ptr<CommandQueue> m_DirectCommandQueue;
	std::shared_ptr<CommandQueue> m_ComputeCommandQueue;
	std::shared_ptr<CommandQueue> m_CopyCommandQueue;

	static unsigned int RenderLoop(void *data);

	void UpdateRenderTargetViews(Microsoft::WRL::ComPtr<ID3D12Device2> device, Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap);

	uint64_t Signal(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, Microsoft::WRL::ComPtr<ID3D12Fence> fence, uint64_t &fenceValue);
};

