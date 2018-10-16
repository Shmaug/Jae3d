#include "Graphics.h"
#include <process.h>

#include "Profiler.h"
#include "Util.h"
#include "CommandQueue.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"

using namespace Microsoft::WRL;
using namespace DirectX;

bool running = false;
HANDLE renderThread;

struct CameraBuffer {
	XMMATRIX ViewProjection;
	XMMATRIX View;
	XMMATRIX Projection;
	XMVECTOR CameraPosition;
} g_CameraBuffer;
struct ObjectBuffer {
	XMMATRIX ObjectToWorld;
	XMMATRIX WorldToObject;
} g_ObjectBuffer;

#pragma region static variable initialization
double Graphics::m_fps = 0;

HWND Graphics::m_hWnd = 0;
RECT Graphics::m_WindowRect;

uint32_t Graphics::m_ClientWidth = 1280;
uint32_t Graphics::m_ClientHeight = 720;

D3D12_VIEWPORT Graphics::m_Viewport;
D3D12_RECT Graphics::m_ScissorRect;

OnRenderDelegate Graphics::OnRender = 0;

HANDLE Graphics::m_mutex = 0;
bool Graphics::m_UseWarp = false;
bool Graphics::m_Initialized = false;
bool Graphics::m_Fullscreen = false;
bool Graphics::m_TearingSupported = false;
bool Graphics::m_VSync = true;
int Graphics::m_fpsCounter = 0;

UINT Graphics::m_RTVDescriptorSize = 0;
UINT Graphics::m_DSVDescriptorSize = 0;
UINT Graphics::m_CurrentBackBufferIndex = 0;

std::shared_ptr<CommandQueue> Graphics::m_DirectCommandQueue;
std::shared_ptr<CommandQueue> Graphics::m_ComputeCommandQueue;
std::shared_ptr<CommandQueue> Graphics::m_CopyCommandQueue;

// DirectX 12 Objects
ComPtr<ID3D12Device2> Graphics::m_Device;
ComPtr<IDXGISwapChain4> Graphics::m_SwapChain;
ComPtr<ID3D12Resource> Graphics::m_RenderBuffers[Graphics::BufferCount];
ComPtr<ID3D12DescriptorHeap> Graphics::m_RTVDescriptorHeap;
ComPtr<ID3D12Resource> Graphics::m_DepthBuffer;
ComPtr<ID3D12DescriptorHeap> Graphics::m_DSVDescriptorHeap;

uint64_t Graphics::m_FenceValues[Graphics::BufferCount];


ComPtr<ID3D12DescriptorHeap> Graphics::m_CBVHeap;
ComPtr<ID3D12Resource> Graphics::m_CameraBuffer;
UINT8* Graphics::m_MappedCameraBuffer;
ComPtr<ID3D12Resource> Graphics::m_ObjectBuffer;
UINT8* Graphics::m_MappedObjectBuffer;
#pragma endregion

#pragma region resource creation
ComPtr<ID3D12Device2> Graphics::CreateDevice(ComPtr<IDXGIAdapter4> adapter) {
	ComPtr<ID3D12Device2> d3d12Device2;
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));

	// Enable debug messages in debug mode.
#if defined(_DEBUG)
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(d3d12Device2.As(&pInfoQueue))) {
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages
		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] = {
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
	}
#endif

	return d3d12Device2;
}
ComPtr<IDXGISwapChain4> Graphics::CreateSwapChain(HWND hWnd, uint32_t width, uint32_t height) {
	ComPtr<IDXGISwapChain4> dxgiSwapChain4;
	ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = BufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
		Graphics::GetCommandQueue()->GetCommandQueue().Get(),
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

	m_CurrentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

	return dxgiSwapChain4;
}
ComPtr<ID3D12DescriptorHeap> Graphics::CreateDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) {
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;

	ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
}

void Graphics::CreateRTVs(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap) {
	auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < BufferCount; ++i) {
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		m_RenderBuffers[i] = backBuffer;
		rtvHandle.Offset(rtvDescriptorSize);
	}
}
#pragma endregion

#pragma region getters
std::shared_ptr<CommandQueue> Graphics::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) {
	switch (type) {
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return m_DirectCommandQueue;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return m_ComputeCommandQueue;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return m_CopyCommandQueue;
	default:
		assert(false && "Invalide command queue type.");
	}
	return nullptr;
}
ComPtr<IDXGIAdapter4> Graphics::GetAdapter(bool useWarp) {
	ComPtr<IDXGIFactory4> dxgiFactory;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter4> dxgiAdapter4;

	if (useWarp) {
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
		ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
	} else {
		SIZE_T maxDedicatedVideoMemory = 0;
		for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			// Check to see if the adapter can create a D3D12 device without actually 
			// creating it. The adapter with the largest dedicated video memory
			// is favored.
			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
				dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory) {
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
	}

	return dxgiAdapter4;
}
bool Graphics::CheckTearingSupport() {
	BOOL allowTearing = FALSE;

	// Rather than create the DXGI 1.5 factory interface directly, we create the
	// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
	// graphics debugging tools which will not support the 1.5 factory interface 
	// until a future update.
	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4)))) {
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5))) {
			if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)))) {
				allowTearing = FALSE;
			}
		}
	}

	return allowTearing == TRUE;
}

D3D12_CPU_DESCRIPTOR_HANDLE Graphics::GetCurrentRenderTargetView() {
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_CurrentBackBufferIndex, m_RTVDescriptorSize);
	return rtv;
}
D3D12_CPU_DESCRIPTOR_HANDLE Graphics::GetDepthStencilView() {
	return m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

bool Graphics::WarpEnabled() {
	return m_UseWarp;
};
bool Graphics::IsInitialized() {
	return m_Initialized;
};
bool Graphics::IsFullscreen() {
	return m_Fullscreen;
};
bool Graphics::TearingSupported() {
	return m_TearingSupported;
};
bool Graphics::VSync() {
	return m_VSync;
};
ComPtr<ID3D12Device2> Graphics::GetDevice() {
	return m_Device;
}
int Graphics::GetAndResetFPS() {
	int x = m_fpsCounter;
	m_fpsCounter = 0;
	return x;
}
#pragma endregion

#pragma region runtime
void Graphics::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), from, to);
	commandList->ResourceBarrier(1, &barrier);
}

void Graphics::SetFullscreen(bool fullscreen) {
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

void Graphics::ResizeDepthBuffer() {
	// depth buffer
	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_ClientWidth, m_ClientHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optimizedClearValue,
		IID_PPV_ARGS(&m_DepthBuffer)
	));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
	dsv.Format = DXGI_FORMAT_D32_FLOAT;
	dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv.Texture2D.MipSlice = 0;
	dsv.Flags = D3D12_DSV_FLAG_NONE;
	m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv, m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}
void Graphics::Resize(uint32_t width, uint32_t height) {
	// Check whether or not we need to resize the swap chain buffers
	if (m_ClientWidth != width || m_ClientHeight != height) {
		// Don't allow 0 size swap chain back buffers.
		m_ClientWidth = std::max(1u, width);
		m_ClientHeight = std::max(1u, height);

		// Flush the GPU queue to make sure the swap chain's back buffers
		// are not being referenced by an in-flight command list.
		Graphics::GetCommandQueue()->Flush();
		
		for (int i = 0; i < BufferCount; ++i) {
			// Any references to the back buffers must be released
			// before the swap chain can be resized.
			m_RenderBuffers[i].Reset();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		ThrowIfFailed(m_SwapChain->GetDesc(&swapChainDesc));
		ThrowIfFailed(m_SwapChain->ResizeBuffers(BufferCount, m_ClientWidth, m_ClientHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		CreateRTVs(m_Device, m_SwapChain, m_RTVDescriptorHeap);

		m_Viewport = CD3DX12_VIEWPORT(0.f, 0.f, (float)m_ClientWidth, (float)m_ClientHeight);

		ResizeDepthBuffer();
	}
}

void Graphics::Initialize(HWND hWnd, bool warp) {
#if defined(_DEBUG)
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif

	m_hWnd = hWnd;
	m_TearingSupported = CheckTearingSupport();
	m_UseWarp = warp;
	ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter(m_UseWarp);

	m_Device = CreateDevice(dxgiAdapter4);

	m_DirectCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_ComputeCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	m_CopyCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COPY);

	m_SwapChain = CreateSwapChain(m_hWnd, m_ClientWidth, m_ClientHeight);

	m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	m_RTVDescriptorHeap = CreateDescriptorHeap(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, BufferCount);
	m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_DSVDescriptorHeap = CreateDescriptorHeap(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
	m_DSVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	CreateRTVs(m_Device, m_SwapChain, m_RTVDescriptorHeap);

	m_ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
	m_Viewport = CD3DX12_VIEWPORT(0.f, 0.f, (float)m_ClientWidth, (float)m_ClientHeight);

	ResizeDepthBuffer();

	// Create a descriptor heap for the constant buffers.
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 2;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		// This flag indicates that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CBVHeap)));

		m_CBVHeap->SetName(L"Constant Buffer View Descriptor Heap");
	}

	// Create the constant buffer.
	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(CameraBuffer) / 256L + 1L) * 256),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_CameraBuffer)));

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ObjectBuffer) / 256L + 1L) * 256L),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_ObjectBuffer)));

	// Describe and create a constant buffer view.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc[2];// = {};
	cbvDesc[0].BufferLocation = m_CameraBuffer->GetGPUVirtualAddress();
	cbvDesc[0].SizeInBytes = (sizeof(CameraBuffer) / 256L + 1L) * 256L;
	cbvDesc[1].BufferLocation = m_ObjectBuffer->GetGPUVirtualAddress();
	cbvDesc[1].SizeInBytes = (sizeof(ObjectBuffer) / 256L + 1L) * 256L;

	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle0(m_CBVHeap->GetCPUDescriptorHandleForHeapStart(), 0, 0);
	m_Device->CreateConstantBufferView(&cbvDesc[0], cbvHandle0);

	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle1(m_CBVHeap->GetCPUDescriptorHandleForHeapStart(), 1, m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	m_Device->CreateConstantBufferView(&cbvDesc[1], cbvHandle1);

	// Initialize and map the constant buffers. We don't unmap this until the
	// app closes. Keeping things mapped for the lifetime of the resource is okay.
	ThrowIfFailed(m_CameraBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedCameraBuffer)));
	memcpy(m_MappedCameraBuffer, &g_CameraBuffer, sizeof(CameraBuffer));

	ThrowIfFailed(m_ObjectBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedObjectBuffer)));
	memcpy(m_MappedObjectBuffer, &g_ObjectBuffer, sizeof(ObjectBuffer));

	m_Initialized = true;
}
void Graphics::StartRenderLoop(HANDLE mutex) {
	running = true;
	m_mutex = mutex;
	renderThread = (HANDLE)_beginthreadex(0, 0, &Graphics::RenderLoop, nullptr, 0, 0);
}

unsigned int __stdcall Graphics::RenderLoop(void *g_this) {
	while (running) {
		WaitForSingleObject(m_mutex, INFINITE);
		m_fpsCounter++;

		// Get current back buffer data
		auto commandQueue = GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto commandList = commandQueue->GetCommandList();

		auto backBuffer = m_RenderBuffers[m_CurrentBackBufferIndex];

		// Draw scene
		TransitionResource(commandList, backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		auto rtv = GetCurrentRenderTargetView();
		auto dsv = m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

		OnRender(commandList);

		// Present
		// Transition back buffer to Present state
		TransitionResource(commandList, backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		// Execute the command list
		m_FenceValues[m_CurrentBackBufferIndex] = commandQueue->Execute(commandList);

		// Release mutex before rendering to prevent vsync from halting main thread
		ReleaseMutex(m_mutex);

		// Present and get the next BackBufferIndex
		UINT flags = m_TearingSupported && !m_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		ThrowIfFailed(m_SwapChain->Present(m_VSync ? 1 : 0, flags));
		m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

		commandQueue->WaitForFenceValue(m_FenceValues[m_CurrentBackBufferIndex]);
	}
	return 0;
}

DXGI_SAMPLE_DESC Graphics::GetMultisampleQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) {
	DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels;
	qualityLevels.Format = format;
	qualityLevels.SampleCount = 1;
	qualityLevels.Flags = flags;
	qualityLevels.NumQualityLevels = 0;

	while (qualityLevels.SampleCount <= numSamples && SUCCEEDED(m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) && qualityLevels.NumQualityLevels > 0) {
		// That works...
		sampleDesc.Count = qualityLevels.SampleCount;
		sampleDesc.Quality = qualityLevels.NumQualityLevels - 1;

		// But can we do better?
		qualityLevels.SampleCount *= 2;
	}

	return sampleDesc;
}

void Graphics::SetShader(ComPtr<ID3D12GraphicsCommandList2> commandList, Shader *shader) {
	commandList->SetPipelineState(shader->m_PipelineState.Get());
	commandList->SetGraphicsRootSignature(shader->m_RootSignature.GetRootSignature().Get());
}
void Graphics::SetCamera(ComPtr<ID3D12GraphicsCommandList2> commandList, Camera *camera) {
	commandList->RSSetViewports(1, &m_Viewport);
	commandList->RSSetScissorRects(1, &m_ScissorRect);

	g_CameraBuffer.View = camera->View();
	g_CameraBuffer.Projection = camera->Projection();
	g_CameraBuffer.ViewProjection = g_CameraBuffer.View * g_CameraBuffer.ViewProjection;
	g_CameraBuffer.CameraPosition = camera->m_Position;
	memcpy(m_MappedCameraBuffer, &g_CameraBuffer, sizeof(CameraBuffer));
}
void Graphics::DrawMesh(ComPtr<ID3D12GraphicsCommandList2> commandList, Mesh* mesh, XMMATRIX modelMatrix) {
	XMVECTOR det = XMMatrixDeterminant(modelMatrix);
	g_ObjectBuffer.WorldToObject = modelMatrix;
	g_ObjectBuffer.ObjectToWorld = XMMatrixInverse(&det, modelMatrix);
	memcpy(m_MappedObjectBuffer, &g_ObjectBuffer, sizeof(ObjectBuffer));

	// set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CBVHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the root descriptor table 0 to the constant buffer descriptor heap
	commandList->SetGraphicsRootDescriptorTable(0, m_CBVHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &mesh->m_VertexBufferView);
	commandList->IASetIndexBuffer(&mesh->m_IndexBufferView);

	commandList->DrawIndexedInstanced(mesh->m_IndexCount, 1, 0, 0, 0);
}

void Graphics::Destroy(){
	running = false;
	::WaitForSingleObject(renderThread, INFINITE);
	::CloseHandle(renderThread);

	GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY)->Flush();
	GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->Flush();
	GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->Flush();
}
#pragma endregion