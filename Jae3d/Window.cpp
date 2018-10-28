#include "Window.h"

#include "Graphics.h"
#include "CommandQueue.h"
#include "Util.h"

using namespace Microsoft::WRL;

Window::Window(HWND hWnd, UINT bufferCount) : m_hWnd(hWnd), m_BufferCount(bufferCount) {
	m_RenderBuffers = new ComPtr<ID3D12Resource>[m_BufferCount];
	m_FenceValues = new uint64_t[m_BufferCount];
	
	ZeroMemory(m_RenderBuffers, sizeof(ComPtr<ID3D12Resource>) * m_BufferCount);
	ZeroMemory(m_FenceValues, sizeof(uint64_t) * m_BufferCount);
	
	m_TearingSupported = Graphics::CheckTearingSupport();

	GetWindowRect(m_hWnd, &m_WindowRect);

	RECT crect;
	GetClientRect(m_hWnd, &crect);
	m_ClientWidth = (uint32_t)(crect.right - crect.left);
	m_ClientHeight = (uint32_t)(crect.bottom - crect.top);

	auto device = Graphics::GetDevice();
	CreateSwapChain();

	m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	m_RTVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DSVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_RTVDescriptorHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_BufferCount);
	m_DSVDescriptorHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	m_msaaRTVDescriptorHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);

	CreateRenderTargets();

	ResizeDepthBuffer();
}
Window::~Window() {
	delete[] m_RenderBuffers;
	delete[] m_FenceValues;
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
	swapChainDesc.Width = m_ClientWidth;
	swapChainDesc.Height = m_ClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1,0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = m_BufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = Graphics::CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
		Graphics::GetCommandQueue()->GetCommandQueue().Get(),
		m_hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

	m_CurrentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

	m_SwapChain = dxgiSwapChain4;
}
void Window::ResizeDepthBuffer() {
	auto device = Graphics::GetDevice();
	// depth buffer
	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_ClientWidth, m_ClientHeight, 1, 1, m_msaaSampleCount, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optimizedClearValue,
		IID_PPV_ARGS(m_DepthBuffer.ReleaseAndGetAddressOf())
	));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
	dsv.Format = DXGI_FORMAT_D32_FLOAT;
	dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	dsv.Texture2D.MipSlice = 0;
	dsv.Flags = D3D12_DSV_FLAG_NONE;
	device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv, m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}
void Window::CreateRenderTargets() {
	auto device = Graphics::GetDevice();

	auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < m_BufferCount; ++i) {
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		m_RenderBuffers[i] = backBuffer;
		rtvHandle.Offset(rtvDescriptorSize);
	}

	D3D12_RESOURCE_DESC msaaRTDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_ClientWidth,
		m_ClientHeight,
		1, // This render target view has only one texture.
		1, // Use a single mipmap level
		m_msaaSampleCount);
	msaaRTDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_CLEAR_VALUE msaaOptimizedClearValue = {};
	msaaOptimizedClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaOptimizedClearValue.DepthStencil = { 1.0f, 0 };
	msaaOptimizedClearValue.Color[0] = 0.0f;
	msaaOptimizedClearValue.Color[1] = 0.0f;
	msaaOptimizedClearValue.Color[2] = 0.0f;
	msaaOptimizedClearValue.Color[3] = 1.0f;

	ThrowIfFailed(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&msaaRTDesc,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		&msaaOptimizedClearValue,
		IID_PPV_ARGS(m_msaaRenderTarget.ReleaseAndGetAddressOf())
	));
	m_msaaRenderTarget->SetName(L"MSAA Render Target");

	D3D12_RENDER_TARGET_VIEW_DESC msaaDesc = {};
	msaaDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	device->CreateRenderTargetView(m_msaaRenderTarget.Get(), &msaaDesc, m_msaaRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void Window::SetFullscreen(bool fullscreen) {
	if (m_Fullscreen == fullscreen) return;
	m_Fullscreen = fullscreen;

	if (m_Fullscreen) // Switching to fullscreen.
	{
		// Store the current window dimensions so they can be restored 
		// when switching out of fullscreen state.
		::GetWindowRect(m_hWnd, &m_WindowRect);

		// Set the window style to a borderless window so the client area fills
		// the entire screen.
		UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

		::SetWindowLongW(m_hWnd, GWL_STYLE, windowStyle);

		// Query the name of the nearest display device for the window.
		// This is required to set the fullscreen dimensions of the window
		// when using a multi-monitor setup.
		HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX monitorInfo = {};
		monitorInfo.cbSize = sizeof(MONITORINFOEX);
		::GetMonitorInfo(hMonitor, &monitorInfo);

		::SetWindowPos(m_hWnd, HWND_TOPMOST,
			monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.top,
			monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		::ShowWindow(m_hWnd, SW_MAXIMIZE);
	} else {
		// Restore all the window decorators.
		::SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

		::SetWindowPos(m_hWnd, HWND_NOTOPMOST,
			m_WindowRect.left,
			m_WindowRect.top,
			m_WindowRect.right - m_WindowRect.left,
			m_WindowRect.bottom - m_WindowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		::ShowWindow(m_hWnd, SW_NORMAL);
	}
}

void Window::Resize() {
	RECT crect;
	GetClientRect(m_hWnd, &crect);
	uint32_t width = (uint32_t)(crect.right - crect.left);
	uint32_t height = (uint32_t)(crect.bottom - crect.top);

	// Check whether or not we need to resize the swap chain buffers
	if (m_ClientWidth != width || m_ClientHeight != height) {
		auto device = Graphics::GetDevice();

		// Don't allow 0 size swap chain back buffers.
		m_ClientWidth = std::max(1u, width);
		m_ClientHeight = std::max(1u, height);

		// Flush the GPU queue to make sure the swap chain's back buffers
		// are not being referenced by an in-flight command list.
		Graphics::GetCommandQueue()->Flush();
		
		for (UINT i = 0; i < m_BufferCount; ++i) {
			// Any references to the back buffers must be released
			// before the swap chain can be resized.
			m_RenderBuffers[i].Reset();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		ThrowIfFailed(m_SwapChain->GetDesc(&swapChainDesc));
		ThrowIfFailed(m_SwapChain->ResizeBuffers(m_BufferCount, m_ClientWidth, m_ClientHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		CreateRenderTargets();

		D3D12_RESOURCE_DESC msaaRTDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R8G8B8A8_UNORM,
			m_ClientWidth,
			m_ClientHeight,
			1, // This render target view has only one texture.
			1, // Use a single mipmap level
			m_msaaSampleCount);
		msaaRTDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_CLEAR_VALUE msaaOptimizedClearValue = {};
		msaaOptimizedClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaOptimizedClearValue.DepthStencil = { 1.0f, 0 };
		msaaOptimizedClearValue.Color[0] = 0.0f;
		msaaOptimizedClearValue.Color[1] = 0.0f;
		msaaOptimizedClearValue.Color[2] = 0.0f;
		msaaOptimizedClearValue.Color[3] = 0.0f;

		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&msaaRTDesc,
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
			&msaaOptimizedClearValue,
			IID_PPV_ARGS(m_msaaRenderTarget.ReleaseAndGetAddressOf())
		));
		m_msaaRenderTarget->SetName(L"MSAA Render Target");

		D3D12_RENDER_TARGET_VIEW_DESC msaaDesc = {};
		msaaDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
		device->CreateRenderTargetView(m_msaaRenderTarget.Get(), &msaaDesc, m_msaaRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		#pragma endregion

		ResizeDepthBuffer();
	}
}
void Window::Close() {

}

D3D12_CPU_DESCRIPTOR_HANDLE Window::GetCurrentRenderTargetView() {
	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrentBackBufferIndex, m_RTVDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_msaaRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, m_RTVDescriptorSize);
	return rtv;
}
D3D12_CPU_DESCRIPTOR_HANDLE Window::GetDepthStencilView() {
	return m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}
ComPtr<ID3D12Resource> Window::GetBackBuffer() {
	return m_RenderBuffers[m_CurrentBackBufferIndex];
}

void Window::PrepareRenderTargets(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	Graphics::TransitionResource(commandList, m_msaaRenderTarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
void Window::PreparePresent(ComPtr<ID3D12GraphicsCommandList2> commandList, std::shared_ptr<CommandQueue> commandQueue) {
	auto backBuffer = GetBackBuffer();

	// Resolve msaa buffers
	Graphics::TransitionResource(commandList, m_msaaRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	Graphics::TransitionResource(commandList, backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST);
	commandList->ResolveSubresource(backBuffer.Get(), 0, m_msaaRenderTarget.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
	Graphics::TransitionResource(commandList, backBuffer.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PRESENT);

	// Execute the command list
	m_FenceValues[m_CurrentBackBufferIndex] = commandQueue->Execute(commandList);
}

void Window::Present(std::shared_ptr<CommandQueue> commandQueue){
	UINT flags = m_TearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
	ThrowIfFailed(m_SwapChain->Present(m_VSync ? 1 : 0, flags));

	m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
	commandQueue->WaitForFenceValue(m_FenceValues[m_CurrentBackBufferIndex]);
}