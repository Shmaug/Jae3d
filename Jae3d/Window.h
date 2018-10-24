#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

class Window {
public:
	Window(HWND hWnd, UINT bufferCount);
	~Window();

	bool IsFullscreen() { return m_Fullscreen; }
	bool VSyncOn() { return m_VSync; }
	void SetVSync(bool vsync) { m_VSync = vsync; }
	void SetFullscreen(bool fullscreen);
	void Resize(uint32_t width, uint32_t height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView();
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView();
	_WRL::ComPtr<ID3D12Resource> GetBackBuffer();

	void PrepareRenderTargets(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);
	void PreparePresent(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, std::shared_ptr<CommandQueue> commandQueue);
	void Present(std::shared_ptr<CommandQueue> commandQueue);

	int GetWidth() { return m_ClientWidth; }
	int GetHeight() { return m_ClientHeight; }

private:
	HWND m_hWnd;
	RECT m_WindowRect;

	bool m_TearingSupported;

	uint32_t m_ClientWidth = 1280;
	uint32_t m_ClientHeight = 720;

	UINT m_msaaSampleCount = 8;
	UINT m_BufferCount = 3;

	bool m_Fullscreen = false;
	bool m_VSync = true;

	UINT m_RTVDescriptorSize;
	UINT m_DSVDescriptorSize;
	UINT m_CurrentBackBufferIndex = 0;

	uint64_t *m_FenceValues = nullptr;

	_WRL::ComPtr<IDXGISwapChain4> m_SwapChain;
	_WRL::ComPtr<ID3D12Resource> *m_RenderBuffers = nullptr;
	_WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
	_WRL::ComPtr<ID3D12Resource> m_DepthBuffer;
	_WRL::ComPtr<ID3D12DescriptorHeap> m_DSVDescriptorHeap;
	_WRL::ComPtr<ID3D12Resource> m_msaaRenderTarget;
	_WRL::ComPtr<ID3D12DescriptorHeap> m_msaaRTVDescriptorHeap;

	void CreateRenderTargets();
	void ResizeDepthBuffer();
	void CreateSwapChain();
};

