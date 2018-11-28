#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <memory>
#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.hpp"

class CommandQueue;
class CommandList;

class Window {
public:
	Window(HWND hWnd, UINT bufferCount);
	~Window();

	bool IsFullscreen() const { return m_Fullscreen; };
	bool VSyncOn() const { return m_VSync; };
	void SetVSync(bool vsync) { m_VSync = vsync; };
	void SetFullscreen(bool fullscreen);
	void Resize();
	void Close();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView();
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView();
	_WRL::ComPtr<ID3D12Resource> GetBackBuffer();

	void PrepareRenderTargets(std::shared_ptr<CommandList> commandList);
	void Present(std::shared_ptr<CommandList> commandList, std::shared_ptr<CommandQueue> commandQueue);
	bool LastFrameCompleted(std::shared_ptr<CommandQueue> commandQueue);

	int GetWidth() const { return m_ClientWidth; }
	int GetHeight() const { return m_ClientHeight; }
	RECT GetRect() const { return m_WindowRect; }
	HWND GetHandle() const { return m_hWnd; }
	UINT GetMSAASamples() const { return m_msaaSampleCount; }

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