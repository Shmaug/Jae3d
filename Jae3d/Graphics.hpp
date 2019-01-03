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
#include "d3dx12.hpp"

class CommandQueue;
class CommandList;
class Game;
class Window;
class SpriteBatch;

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
	JAE_API static std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
	static bool IsInitialized() { return mInitialized; }
	static _WRL::ComPtr<ID3D12Device2> GetDevice() { return mDevice; }
	JAE_API static std::shared_ptr<Window> GetWindow();
	JAE_API static std::shared_ptr<SpriteBatch> GetSpriteBatch();

	JAE_API static void Initialize(HWND hWnd, unsigned int bufferCount);
	JAE_API static void Destroy();
	JAE_API static bool FrameReady();

	JAE_API static unsigned int BufferCount();
	JAE_API static unsigned int CurrentFrameIndex();

	JAE_API static _WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	JAE_API static UINT DescriptorIncrement(D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	JAE_API static bool CheckTearingSupport();
	JAE_API static DXGI_SAMPLE_DESC GetSupportedMSAAQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags);
	
	JAE_API static void UploadData(std::shared_ptr<CommandList> commandList, ID3D12Resource** dst, ID3D12Resource** intermediate, size_t count, size_t stride, const void* data);

private:
	// Set to true once the DX12 objects have been initialized.
	JAE_API static bool mInitialized;
	JAE_API static std::shared_ptr<SpriteBatch> mSpriteBatch;

	JAE_API static std::shared_ptr<Window> mWindow;

	JAE_API static std::shared_ptr<CommandQueue> mDirectCommandQueue;
	JAE_API static std::shared_ptr<CommandQueue> mComputeCommandQueue;
	JAE_API static std::shared_ptr<CommandQueue> mCopyCommandQueue;

	// DirectX 12 Objects
	JAE_API static _WRL::ComPtr<ID3D12Device2> mDevice;

	JAE_API static _WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool useWarp);
	JAE_API static _WRL::ComPtr<ID3D12Device2> CreateDevice();
};