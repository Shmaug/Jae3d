#pragma once

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
#define _WRL Microsoft::WRL

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <memory>
#include <functional>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

class CommandQueue;
class Game;
class Mesh;
class Shader;
class Camera;

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

using OnRenderDelegate = std::function<void(_WRL::ComPtr<ID3D12GraphicsCommandList2>)>;

class Graphics {
public:
	static const int BufferCount = 3;

	static double m_fps;

	static HWND m_hWnd;
	static RECT m_WindowRect;

	static uint32_t m_ClientWidth;
	static uint32_t m_ClientHeight;

	static D3D12_VIEWPORT m_Viewport;
	static D3D12_RECT m_ScissorRect;

	static OnRenderDelegate OnRender;

	static std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	static bool WarpEnabled();
	static bool IsInitialized();
	static bool IsFullscreen();
	static bool TearingSupported();
	static bool VSync();
	static _WRL::ComPtr<ID3D12Device2> GetDevice();

	static void SetVSync(bool vsync) { m_VSync = vsync; }
	static int GetAndResetFPS();
	static void SetFullscreen(bool fullscreen);

	static void Initialize(HWND hWnd);
	static void Destroy();
	static void Resize(uint32_t width, uint32_t height);
	static void StartRenderLoop(HANDLE mutex);
	static void TransitionResource(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, _WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
	
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView();
	static D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView();

	static void Graphics::SetCamera(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, Camera* camera);
	static void Graphics::SetShader(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, Shader* shader);
	static void Graphics::DrawMesh(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, Mesh* mesh, DirectX::XMMATRIX modelMatrix);
	static DXGI_SAMPLE_DESC GetMultisampleQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags);

private:
	static HANDLE m_mutex;
	// Use WARP adapter
	static bool m_UseWarp;
	// Set to true once the DX12 objects have been initialized.
	static bool m_Initialized;
	static bool m_Fullscreen;
	static bool m_TearingSupported;
	static bool m_VSync;
	static int m_fpsCounter;

	static UINT m_RTVDescriptorSize;
	static UINT m_DSVDescriptorSize;
	static UINT m_CurrentBackBufferIndex;

	static std::shared_ptr<CommandQueue> m_DirectCommandQueue;
	static std::shared_ptr<CommandQueue> m_ComputeCommandQueue;
	static std::shared_ptr<CommandQueue> m_CopyCommandQueue;

	// DirectX 12 Objects
	static _WRL::ComPtr<ID3D12Device2> m_Device;
	static _WRL::ComPtr<IDXGISwapChain4> m_SwapChain;
	static _WRL::ComPtr<ID3D12Resource> m_RenderBuffers[BufferCount];
	static _WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
	static _WRL::ComPtr<ID3D12Resource> m_DepthBuffer;
	static _WRL::ComPtr<ID3D12DescriptorHeap> m_DSVDescriptorHeap;

	static uint64_t m_FenceValues[Graphics::BufferCount];

	static _WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;
	static _WRL::ComPtr<ID3D12Resource> m_CameraBuffer;
	static _WRL::ComPtr<ID3D12Resource> m_ObjectBuffer;
	static UINT8* m_MappedCameraBuffer;
	static UINT8* m_MappedObjectBuffer;

	static bool CheckTearingSupport();
	static _WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);

	static _WRL::ComPtr<ID3D12Device2> CreateDevice();
	static _WRL::ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd, uint32_t width, uint32_t height);
	static _WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(_WRL::ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
	
	static void CreateRTVs(_WRL::ComPtr<ID3D12Device2> device, _WRL::ComPtr<IDXGISwapChain4> swapChain, _WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap);

	static void ResizeDepthBuffer();

	static unsigned int RenderLoop(void *data);
};