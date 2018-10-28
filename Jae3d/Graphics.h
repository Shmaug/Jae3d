#pragma once

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
class Window;

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
	static double m_fps;

	static D3D12_RECT m_ScissorRect;

	static OnRenderDelegate OnRender;
	
	static std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	static bool IsInitialized() { return m_Initialized; }
	static _WRL::ComPtr<ID3D12Device2> GetDevice() { return m_Device; }
	static std::shared_ptr<Window> GetWindow() { return m_Window; }
	static UINT GetMSAASamples();

	static int GetAndResetFPS();

	static void Initialize(HWND hWnd);
	static void Destroy();
	static void StartRenderLoop(HANDLE mutex);

	static void TransitionResource(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, _WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
	static _WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
	
	static bool CheckTearingSupport();
	static DXGI_SAMPLE_DESC GetSupportedMSAAQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags);

	static void Graphics::SetCamera(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, Camera* camera);
	static void Graphics::SetShader(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, Shader* shader);
	static void Graphics::DrawMesh(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, Mesh* mesh, DirectX::XMMATRIX modelMatrix);
	static void Graphics::DrawMesh(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, Mesh* mesh);

private:
	static HANDLE m_Mutex;
	// Set to true once the DX12 objects have been initialized.
	static bool m_Initialized;
	static int m_fpsCounter;

	static std::shared_ptr<Window> m_Window;

	static std::shared_ptr<CommandQueue> m_DirectCommandQueue;
	static std::shared_ptr<CommandQueue> m_ComputeCommandQueue;
	static std::shared_ptr<CommandQueue> m_CopyCommandQueue;

	// DirectX 12 Objects
	static _WRL::ComPtr<ID3D12Device2> m_Device;

	static _WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;
	static _WRL::ComPtr<ID3D12Resource> m_CameraBuffer;
	static _WRL::ComPtr<ID3D12Resource> m_ObjectBuffer;
	static UINT8* m_MappedCameraBuffer;
	static UINT8* m_MappedObjectBuffer;

	static _WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);

	static _WRL::ComPtr<ID3D12Device2> CreateDevice();
	
	static unsigned int RenderLoop(void *data);
};