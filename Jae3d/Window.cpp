#include "Window.hpp"

#include "Graphics.hpp"
#include "CommandQueue.hpp"

#include "CommandList.hpp"

using namespace Microsoft::WRL;
using namespace std;

Window::Window(HWND hWnd, UINT bufferCount) : mhWnd(hWnd), mBufferCount(bufferCount) {
	mRenderBuffers = new ComPtr<ID3D12Resource>[mBufferCount];
	mFenceValues = new uint64_t[mBufferCount];
	
	ZeroMemory(mRenderBuffers, sizeof(ComPtr<ID3D12Resource>) * mBufferCount);
	ZeroMemory(mFenceValues, sizeof(uint64_t) * mBufferCount);
	
	mTearingSupported = Graphics::CheckTearingSupport();

	GetWindowRect(mhWnd, &mWindowRect);

	RECT crect;
	GetClientRect(mhWnd, &crect);
	mClientWidth = (uint32_t)(crect.right - crect.left);
	mClientHeight = (uint32_t)(crect.bottom - crect.top);

	HDC hdc = GetDC(hWnd);
	mLogPixelsX = GetDeviceCaps(hdc, LOGPIXELSX);
	mLogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(hWnd, hdc);

	auto device = Graphics::GetDevice();
	CreateSwapChain();

	mRTVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDSVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	mRTVDescriptorHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, mBufferCount);
	mDSVDescriptorHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	mMSAADescriptorHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);

	CreateBuffers();
}
Window::~Window() {
	delete[] mRenderBuffers;
	delete[] mFenceValues;
}

void Window::CreateSwapChain() {
	ComPtr<IDXGISwapChain4> dxgiSwapChain4;
	ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = mClientWidth;
	swapChainDesc.Height = mClientHeight;
	swapChainDesc.Format = mDisplayFormat;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1,0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = mBufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = Graphics::CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
		Graphics::GetCommandQueue()->GetCommandQueue().Get(),
		mhWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(mhWnd, DXGI_MWA_NO_ALT_ENTER));
	ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));
	mCurrentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();
	mSwapChain = dxgiSwapChain4;
}

void Window::CreateBuffers() {
	auto device = Graphics::GetDevice();

#pragma region render buffers
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < mBufferCount; ++i) {
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		mRenderBuffers[i] = backBuffer;
		rtvHandle.Offset(mRTVDescriptorSize);
	}
#pragma endregion

#pragma region msaa buffer
	D3D12_RESOURCE_DESC msaaRTDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		mDisplayFormat,
		mClientWidth,
		mClientHeight,
		1, // This render target view has only one texture.
		1, // Use a single mipmap level
		mmsaaSampleCount);
	msaaRTDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE msaaOptimizedClearValue = {};
	msaaOptimizedClearValue.Format = mDisplayFormat;
	msaaOptimizedClearValue.DepthStencil = { 1.0f, 0 };
	msaaOptimizedClearValue.Color[0] = 0.0f;
	msaaOptimizedClearValue.Color[1] = 0.0f;
	msaaOptimizedClearValue.Color[2] = 0.0f;
	msaaOptimizedClearValue.Color[3] = 1.0f;

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&msaaRTDesc,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		&msaaOptimizedClearValue,
		IID_PPV_ARGS(mMSAATarget.ReleaseAndGetAddressOf())
	));
	mMSAATarget->SetName(L"MSAA Render Target");

	D3D12_RENDER_TARGET_VIEW_DESC msaaDesc = {};
	msaaDesc.Format = mDisplayFormat;
	msaaDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	device->CreateRenderTargetView(mMSAATarget.Get(), &msaaDesc, mMSAADescriptorHeap->GetCPUDescriptorHandleForHeapStart());
#pragma endregion

#pragma region depth buffer
	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = mDepthFormat;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(mDepthFormat, mClientWidth, mClientHeight, 1, 1, mmsaaSampleCount, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optimizedClearValue,
		IID_PPV_ARGS(mDepthBuffer.ReleaseAndGetAddressOf())
	));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
	dsv.Format = mDepthFormat;
	dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	dsv.Texture2D.MipSlice = 0;
	dsv.Flags = D3D12_DSV_FLAG_NONE;
	device->CreateDepthStencilView(mDepthBuffer.Get(), &dsv, mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
#pragma endregion

}
void Window::SetFullscreen(bool fullscreen) {
	if (mFullscreen == fullscreen) return;
	mFullscreen = fullscreen;

	if (mFullscreen) // Switching to fullscreen.
	{
		// Store the current window dimensions so they can be restored 
		// when switching out of fullscreen state.
		GetWindowRect(mhWnd, &mWindowRect);

		// Set the window style to a borderless window so the client area fills
		// the entire screen.
		UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

		SetWindowLongW(mhWnd, GWL_STYLE, windowStyle);

		// Query the name of the nearest display device for the window.
		// This is required to set the fullscreen dimensions of the window
		// when using a multi-monitor setup.
		HMONITOR hMonitor = ::MonitorFromWindow(mhWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX monitorInfo = {};
		monitorInfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(hMonitor, &monitorInfo);

		SetWindowPos(mhWnd, HWND_TOPMOST,
			monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.top,
			monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(mhWnd, SW_MAXIMIZE);
	} else {
		// Restore all the window decorators.
		SetWindowLong(mhWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

		SetWindowPos(mhWnd, HWND_NOTOPMOST,
			mWindowRect.left,
			mWindowRect.top,
			mWindowRect.right - mWindowRect.left,
			mWindowRect.bottom - mWindowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(mhWnd, SW_NORMAL);
	}
}

void Window::Resize() {
	HDC hdc = GetDC(mhWnd);
	mLogPixelsX = GetDeviceCaps(hdc, LOGPIXELSX);
	mLogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(mhWnd, hdc);

	RECT crect;
	GetClientRect(mhWnd, &crect);
	uint32_t width = (uint32_t)(crect.right - crect.left);
	uint32_t height = (uint32_t)(crect.bottom - crect.top);

	// Check whether or not we need to resize the swap chain buffers
	if (mClientWidth != width || mClientHeight != height) {
		auto device = Graphics::GetDevice();

		// Don't allow 0 size swap chain back buffers.
		mClientWidth = max(1u, width);
		mClientHeight = max(1u, height);

		// Flush the GPU queue to make sure the swap chain's back buffers
		// are not being referenced by an in-flight command list.
		Graphics::GetCommandQueue()->Flush();
		
		for (UINT i = 0; i < mBufferCount; ++i) {
			// Any references to the back buffers must be released
			// before the swap chain can be resized.
			mRenderBuffers[i].Reset();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		ThrowIfFailed(mSwapChain->GetDesc(&swapChainDesc));
		ThrowIfFailed(mSwapChain->ResizeBuffers(mBufferCount, mClientWidth, mClientHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		CreateBuffers();
	}
}
void Window::Close() {
	PostQuitMessage(0);
}

D3D12_CPU_DESCRIPTOR_HANDLE Window::GetCurrentRenderTargetView() {
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mMSAADescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, mRTVDescriptorSize);
	return rtv;
}
D3D12_CPU_DESCRIPTOR_HANDLE Window::GetDepthStencilView() {
	return mDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void Window::PrepareRenderTargets(std::shared_ptr<CommandList> commandList) {
	commandList->TransitionResource(mMSAATarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void Window::Present(std::shared_ptr<CommandList> commandList, shared_ptr<CommandQueue> commandQueue){
	auto backBuffer = mRenderBuffers[mCurrentBackBufferIndex];

	auto d3dCommmandList = commandList->D3DCommandList();

	// Resolve msaa buffers
	commandList->TransitionResource(mMSAATarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	commandList->TransitionResource(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST);
	d3dCommmandList->ResolveSubresource(backBuffer.Get(), 0, mMSAATarget.Get(), 0, mDisplayFormat);
	commandList->TransitionResource(backBuffer.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PRESENT);

	// Execute the command list
	mFenceValues[mCurrentBackBufferIndex] = commandQueue->Execute(commandList);

	UINT syncInterval = mVSync ? 1 : 0;
	UINT flags = mTearingSupported && !mVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
	ThrowIfFailed(mSwapChain->Present(syncInterval, flags));
	mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
}

bool Window::LastFrameCompleted(shared_ptr<CommandQueue> commandQueue) {
	return commandQueue->IsFenceComplete(mFenceValues[mCurrentBackBufferIndex]);
}
