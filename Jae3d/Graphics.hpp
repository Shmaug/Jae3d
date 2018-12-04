#pragma once

#include "Util.hpp"

#include <wrl.h>

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <memory>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "../Common/d3dx12.hpp"

class CommandQueue;
class CommandList;
class Game;
class Window;

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

class Graphics {
public:
	static std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	static bool IsInitialized() { return mInitialized; }
	static _WRL::ComPtr<ID3D12Device2> GetDevice() { return mDevice; }
	static std::shared_ptr<Window> GetWindow() { return mWindow; }
	static UINT GetMSAASamples();

	static void Initialize(HWND hWnd);
	static void Destroy();
	static bool FrameReady();

	static _WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	
	static bool CheckTearingSupport();
	static DXGI_FORMAT GetDisplayFormat();
	static DXGI_FORMAT GetDepthFormat();
	static DXGI_SAMPLE_DESC GetSupportedMSAAQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags);
	
	static void UploadData(std::shared_ptr<CommandList> commandList, ID3D12Resource** dst, ID3D12Resource** intermediate,
		size_t count, size_t stride, const void* data);

private:
	// Set to true once the DX12 objects have been initialized.
	static bool mInitialized;

	static std::shared_ptr<Window> mWindow;

	static std::shared_ptr<CommandQueue> mDirectCommandQueue;
	static std::shared_ptr<CommandQueue> mComputeCommandQueue;
	static std::shared_ptr<CommandQueue> mCopyCommandQueue;

	// DirectX 12 Objects
	static _WRL::ComPtr<ID3D12Device2> mDevice;

	static _WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);

	static _WRL::ComPtr<ID3D12Device2> CreateDevice();
};