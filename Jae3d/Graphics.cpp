#include "Graphics.hpp"

#include "Profiler.hpp"
#include "CommandQueue.hpp"
#include "Window.hpp"

#include "CommandList.hpp"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

#ifdef _DEBUG
#include <dxgidebug.h>
ComPtr<IDXGIDebug1> dxgiDebugInterface;
#endif

#pragma region static variable initialization
bool Graphics::mInitialized = false;

shared_ptr<Window> Graphics::mWindow;

shared_ptr<CommandQueue> Graphics::mDirectCommandQueue;
shared_ptr<CommandQueue> Graphics::mComputeCommandQueue;
shared_ptr<CommandQueue> Graphics::mCopyCommandQueue;

// DirectX 12 Objects
ComPtr<ID3D12Device2> Graphics::mDevice;
#pragma endregion

#pragma region resource creation
ComPtr<ID3D12Device2> Graphics::CreateDevice() {
	ComPtr<IDXGIAdapter4> adapter = GetAdapter(false);

	ComPtr<ID3D12Device2> d3d12Device2;
	HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2));
#ifdef _DEBUG
	if (FAILED(hr)) {
		ComPtr<IDXGIAdapter> warpAdapter = GetAdapter(true);
		hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2));
	}
#endif
	ThrowIfFailed(hr);

	
	// Enable debug messages in debug mode.
#ifdef _DEBUG
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
ComPtr<ID3D12DescriptorHeap> Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags) {
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	desc.Flags = flags;

	ThrowIfFailed(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
}
#pragma endregion

#pragma region getters
shared_ptr<CommandQueue> Graphics::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) {
	switch (type) {
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return mDirectCommandQueue;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return mComputeCommandQueue;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return mCopyCommandQueue;
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

DXGI_FORMAT Graphics::GetDisplayFormat() { return mWindow->GetDisplayFormat(); }
DXGI_FORMAT Graphics::GetDepthFormat() { return mWindow->GetDepthFormat(); }

unsigned int Graphics::BufferCount() { return mWindow->BufferCount(); }
unsigned int Graphics::CurrentFrameIndex() { return mWindow->CurrentFrameIndex(); }

UINT Graphics::GetMSAASamples() {
	return mWindow->GetMSAASamples();
}
#pragma endregion

#pragma region runtime

void Graphics::Initialize(HWND hWnd, unsigned int bufferCount) {
#ifdef _DEBUG
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
	
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebugInterface));
#endif

	mDevice = CreateDevice();

	mDirectCommandQueue = shared_ptr<CommandQueue>(new CommandQueue(mDevice, D3D12_COMMAND_LIST_TYPE_DIRECT));
	mComputeCommandQueue = shared_ptr<CommandQueue>(new CommandQueue(mDevice, D3D12_COMMAND_LIST_TYPE_COMPUTE));
	mCopyCommandQueue = shared_ptr<CommandQueue>(new CommandQueue(mDevice, D3D12_COMMAND_LIST_TYPE_COPY));

	mWindow = shared_ptr<Window>(new Window(hWnd, bufferCount));

	mInitialized = true;
}

bool Graphics::FrameReady() {
	return mWindow->LastFrameCompleted(GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
}

DXGI_SAMPLE_DESC Graphics::GetSupportedMSAAQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) {
	DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels;
	qualityLevels.Format = format;
	qualityLevels.SampleCount = 1;
	qualityLevels.Flags = flags;
	qualityLevels.NumQualityLevels = 0;

	/*
	while (qualityLevels.SampleCount <= numSamples &&
		SUCCEEDED(mDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) &&
		qualityLevels.NumQualityLevels > 0) {

		sampleDesc.Count = qualityLevels.SampleCount;
		sampleDesc.Quality = qualityLevels.NumQualityLevels - 1;

		qualityLevels.SampleCount *= 2;
	}
	*/
	qualityLevels.SampleCount = numSamples;

	return sampleDesc;
}

void Graphics::Destroy(){
	mDirectCommandQueue->Flush();
	mComputeCommandQueue->Flush();
	mCopyCommandQueue->Flush();

	mDirectCommandQueue.reset();
	mComputeCommandQueue.reset();
	mCopyCommandQueue.reset();

	mWindow.reset();

	mDevice.Reset();

#ifdef _DEBUG
	//dxgiDebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif
}
#pragma endregion

void Graphics::UploadData(shared_ptr<CommandList> commandList,
	ID3D12Resource** dst, ID3D12Resource** intermediate,
	size_t count, size_t stride, const void* data) {

	auto device = GetDevice();

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(stride * count),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(dst)));

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(count * stride),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(intermediate)));

	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = data;
	subresourceData.RowPitch = count * stride;
	subresourceData.SlicePitch = subresourceData.RowPitch;

	UpdateSubresources(commandList->D3DCommandList().Get(), *dst, *intermediate, 0, 0, 1, &subresourceData);
}