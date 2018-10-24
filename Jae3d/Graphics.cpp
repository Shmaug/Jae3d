#include "Graphics.h"
#include <process.h>

#include "Profiler.h"
#include "Util.h"
#include "RootSignature.h"
#include "CommandQueue.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Window.h"

using namespace Microsoft::WRL;
using namespace DirectX;

bool running = false;
HANDLE renderThread;

struct CameraBuffer {
	XMFLOAT4X4 ViewProjection;
	XMFLOAT4X4 View;
	XMFLOAT4X4 Projection;
	XMFLOAT3 CameraPosition;

	static UINT64 AlignedSize() {
		int s = sizeof(XMFLOAT4X4) * 3 + sizeof(XMFLOAT3);
		return (UINT64)((sizeof(CameraBuffer) + 257) / 256) * 256;
	}
} g_CameraBufferData;
struct ObjectBuffer {
	XMFLOAT4X4 ObjectToWorld;
	XMFLOAT4X4 WorldToObject;

	static UINT64 AlignedSize() {
		int s = sizeof(XMFLOAT4X4) * 3 + sizeof(XMFLOAT3);
		return (UINT64)((sizeof(ObjectBuffer) + 257) / 256) * 256;
	}
} g_ObjectBufferData;

#pragma region static variable initialization
double Graphics::m_fps = 0;
int Graphics::m_fpsCounter;

D3D12_RECT Graphics::m_ScissorRect;

OnRenderDelegate Graphics::OnRender;

HANDLE Graphics::m_Mutex;
bool Graphics::m_Initialized = false;

std::shared_ptr<Window> Graphics::m_Window;
std::shared_ptr<CommandQueue> Graphics::m_DirectCommandQueue;
std::shared_ptr<CommandQueue> Graphics::m_ComputeCommandQueue;
std::shared_ptr<CommandQueue> Graphics::m_CopyCommandQueue;

// DirectX 12 Objects
ComPtr<ID3D12Device2> Graphics::m_Device;

ComPtr<ID3D12DescriptorHeap> Graphics::m_CbvHeap;
ComPtr<ID3D12Resource> Graphics::m_CameraBuffer;
UINT8* Graphics::m_MappedCameraBuffer;
ComPtr<ID3D12Resource> Graphics::m_ObjectBuffer;
UINT8* Graphics::m_MappedObjectBuffer;
#pragma endregion

#pragma region resource creation
ComPtr<ID3D12Device2> Graphics::CreateDevice() {
	ComPtr<IDXGIAdapter4> adapter = GetAdapter(false);

	ComPtr<ID3D12Device2> d3d12Device2;
	HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2));
#if defined(_DEBUG)
	if (FAILED(hr)) {
		ComPtr<IDXGIAdapter> warpAdapter = GetAdapter(true);
		hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2));
	}
#endif
	ThrowIfFailed(hr);

	
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
ComPtr<ID3D12DescriptorHeap> Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) {
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
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

int Graphics::GetAndResetFPS() {
	int x = m_fpsCounter;
	m_fpsCounter = 0;
	return x;
}
UINT Graphics::GetMSAASamples() {
	return m_Window->GetMSAASamples();
}
#pragma endregion

#pragma region runtime
void Graphics::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to) {
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), from, to);
	commandList->ResourceBarrier(1, &barrier);
}

void Graphics::Initialize(HWND hWnd) {
#if defined(_DEBUG)
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif

	m_Device = CreateDevice();

	m_DirectCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_ComputeCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	m_CopyCommandQueue = std::make_shared<CommandQueue>(m_Device, D3D12_COMMAND_LIST_TYPE_COPY);

	m_Window = std::make_shared<Window>(hWnd, 3);

	m_ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);

	#pragma region Constant buffers
	// Create a descriptor heap for the constant buffers.
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 2;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		// This flag indicates that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_CbvHeap)));

		m_CbvHeap->SetName(L"CBV Descriptor Heap");
	}
	ZeroMemory(&g_ObjectBufferData, sizeof(g_ObjectBufferData));
	ZeroMemory(&g_CameraBufferData, sizeof(g_CameraBufferData));

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(ObjectBuffer::AlignedSize()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_ObjectBuffer)));

	m_ObjectBuffer->SetName(L"CB Object");

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(CameraBuffer::AlignedSize()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_CameraBuffer)));
	m_CameraBuffer->SetName(L"CB Camera");

	// Describe and create a constant buffer view.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc[2];
	cbvDesc[0].BufferLocation = m_ObjectBuffer->GetGPUVirtualAddress();
	cbvDesc[0].SizeInBytes = (UINT)ObjectBuffer::AlignedSize();
	cbvDesc[1].BufferLocation = m_CameraBuffer->GetGPUVirtualAddress();
	cbvDesc[1].SizeInBytes = (UINT)CameraBuffer::AlignedSize();

	D3D12_CPU_DESCRIPTOR_HANDLE desch = m_CbvHeap->GetCPUDescriptorHandleForHeapStart();
	UINT descs = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle0(desch, 0, descs);
	m_Device->CreateConstantBufferView(&cbvDesc[0], cbvHandle0);

	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle1(desch, 1, descs);
	m_Device->CreateConstantBufferView(&cbvDesc[1], cbvHandle1);

	// Map the constant buffers.
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_ObjectBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedObjectBuffer)));
	ThrowIfFailed(m_CameraBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_MappedCameraBuffer)));
	ZeroMemory(m_MappedObjectBuffer, cbvDesc[0].SizeInBytes);
	ZeroMemory(m_MappedCameraBuffer, cbvDesc[1].SizeInBytes);
	#pragma endregion

	m_Initialized = true;
}
void Graphics::StartRenderLoop(HANDLE mutex) {
	running = true;
	m_Mutex = mutex;
	renderThread = (HANDLE)_beginthreadex(0, 0, &Graphics::RenderLoop, nullptr, 0, 0);
}

unsigned int __stdcall Graphics::RenderLoop(void *g_this) {
	while (running) {
		WaitForSingleObject(m_Mutex, INFINITE);
		m_fpsCounter++;

		// Get current back buffer data
		auto commandQueue = GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto commandList = commandQueue->GetCommandList();
		auto backBuffer = m_Window->GetBackBuffer();

		// Draw scene
		m_Window->PrepareRenderTargets(commandList);

		auto rtv = m_Window->GetCurrentRenderTargetView();
		auto dsv = m_Window->GetDepthStencilView();
		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

		OnRender(commandList);

		// Present
		m_Window->PreparePresent(commandList, commandQueue);

		// Release mutex before rendering to prevent vsync from halting main thread
		ReleaseMutex(m_Mutex);

		m_Window->Present(commandQueue);
	}
	return 0;
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
		SUCCEEDED(m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) &&
		qualityLevels.NumQualityLevels > 0) {

		sampleDesc.Count = qualityLevels.SampleCount;
		sampleDesc.Quality = qualityLevels.NumQualityLevels - 1;

		qualityLevels.SampleCount *= 2;
	}
	*/
	qualityLevels.SampleCount = numSamples;

	return sampleDesc;
}

void Graphics::SetShader(ComPtr<ID3D12GraphicsCommandList2> commandList, Shader *shader) {
	commandList->SetGraphicsRootSignature(shader->m_RootSignature->GetRootSignature().Get());
	commandList->SetPipelineState(shader->m_PipelineState.Get());
}
void Graphics::SetCamera(ComPtr<ID3D12GraphicsCommandList2> commandList, Camera *camera) {
	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.f, 0.f, (float)m_Window->GetWidth(), (float)m_Window->GetHeight());
	commandList->RSSetViewports(1, &vp);
	commandList->RSSetScissorRects(1, &m_ScissorRect);

	XMStoreFloat4x4(&g_CameraBufferData.View, camera->View());
	XMStoreFloat4x4(&g_CameraBufferData.Projection, camera->Projection());
	XMStoreFloat4x4(&g_CameraBufferData.ViewProjection, camera->View() * camera->Projection());
	XMStoreFloat3(&g_CameraBufferData.CameraPosition, camera->m_Position);
}
void Graphics::DrawMesh(ComPtr<ID3D12GraphicsCommandList2> commandList, Mesh* mesh, XMMATRIX modelMatrix) {
	XMVECTOR det = XMMatrixDeterminant(modelMatrix);
	XMStoreFloat4x4(&g_ObjectBufferData.ObjectToWorld, modelMatrix);
	XMStoreFloat4x4(&g_ObjectBufferData.WorldToObject, XMMatrixInverse(&det, modelMatrix));

	memcpy(m_MappedObjectBuffer, &g_ObjectBufferData, sizeof(ObjectBuffer));
	memcpy(m_MappedCameraBuffer, &g_CameraBufferData, sizeof(CameraBuffer));

	// set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the root descriptor table 0 to the constant buffer descriptor heap
	commandList->SetGraphicsRootDescriptorTable(0, m_CbvHeap->GetGPUDescriptorHandleForHeapStart());

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

	// TODO: call Reset() on all resource ComPtrs
}
#pragma endregion