#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <memory>
#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.hpp"

class CommandQueue;
class CommandList;

class Window {
public:
	JAE_API Window(HWND hWnd, UINT bufferCount);
	JAE_API ~Window();

	bool IsFullscreen() const { return mFullscreen; };
	bool VSyncOn() const { return mVSync; };
	void SetVSync(bool vsync) { mVSync = vsync; };
	JAE_API void SetFullscreen(bool fullscreen);
	JAE_API void Resize();
	JAE_API void Close();

	JAE_API D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView();

	JAE_API void PrepareRender(std::shared_ptr<CommandList> commandList);
	JAE_API void Present(std::shared_ptr<CommandList> commandList, std::shared_ptr<CommandQueue> commandQueue);
	JAE_API bool LastFrameCompleted(std::shared_ptr<CommandQueue> commandQueue);

	_WRL::ComPtr<ID3D12Resource> RenderBuffer() const { return mRenderBuffers[mCurrentBackBufferIndex]; }

	unsigned int CurrentBuffer() const { return mCurrentBackBufferIndex; }
	unsigned int BufferCount() const { return mBufferCount; }
	unsigned int CurrentFrameIndex() const { return mCurrentBackBufferIndex; }

	int GetWidth() const { return mClientWidth; }
	int GetHeight() const { return mClientHeight; }

	UINT GetLogPixelsX() const { return mLogPixelsX; }
	UINT GetLogPixelsY() const { return mLogPixelsY; }

	RECT GetRect() { GetWindowRect(mhWnd, &mWindowRect); return mWindowRect; }
	HWND GetHandle() const { return mhWnd; }

private:
	HWND mhWnd;
	RECT mWindowRect;

	bool mTearingSupported;

	uint32_t mClientWidth = 1280;
	uint32_t mClientHeight = 720;
	UINT mLogPixelsX;
	UINT mLogPixelsY;

	UINT mBufferCount = 3;

	bool mFullscreen = false;
	bool mVSync = true;
	DXGI_FORMAT mDisplayFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	UINT mRTVDescriptorSize;
	UINT mCurrentBackBufferIndex = 0;

	uint64_t* mFenceValues = nullptr;

	_WRL::ComPtr<IDXGISwapChain4> mSwapChain;

	_WRL::ComPtr<ID3D12Resource>* mRenderBuffers = nullptr;
	_WRL::ComPtr<ID3D12DescriptorHeap> mRTVDescriptorHeap;

	JAE_API void CreateBuffers();
	JAE_API void CreateSwapChain();
};