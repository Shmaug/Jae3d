#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>

#include "Util.hpp"
#include <d3d12.h>
#include "d3dx12.hpp"

#include "Object.hpp"
#include <wrl.h>

class CommandList;
class ConstantBuffer;
class Texture;
class Shader;
class Scene;
class Light;

class Camera : public Object {
public:
	JAE_API Camera(jwstring name);
	JAE_API ~Camera();

	DirectX::XMFLOAT4 PerspectiveBounds() const { return mPerspectiveBounds; }
	float FieldOfView() const { return mFieldOfView; }
	float Near() const { return mNear; }
	float Far() const { return mFar; }
	unsigned int PixelWidth() const { return mPixelWidth; }
	unsigned int PixelHeight() const { return mPixelHeight; }

	void PerspectiveBounds(DirectX::XMFLOAT4 f) { mPerspectiveBounds = f; mFieldOfView = 0; mTransformDirty = true; }
	void FieldOfView(float f) { mPerspectiveBounds = { 0,0,0,0 }; mFieldOfView = f; mTransformDirty = true; }
	void Near(float n) { mNear = n; mTransformDirty = true; }
	void Far(float f) { mFar = f; mTransformDirty = true; }
	void PixelWidth(unsigned int w) { mPixelWidth = w; mTransformDirty = true; }
	void PixelHeight(unsigned int h) { mPixelHeight = h; mTransformDirty = true; }

	DirectX::XMFLOAT4X4 View() { UpdateTransform(); return mView; }
	DirectX::XMFLOAT4X4 Projection() { UpdateTransform(); return mProjection; }
	DirectX::XMFLOAT4X4 ViewProjection() { UpdateTransform(); return mViewProjection; }
	DirectX::BoundingFrustum Frustum() { UpdateTransform(); return mFrustum; }
	std::shared_ptr<ConstantBuffer> CBuffer() { return mCBuffer; }

	DXGI_FORMAT RenderFormat() const { return mRenderFormat; }
	DXGI_FORMAT DepthFormat() const { return mDepthFormat; }
	_WRL::ComPtr<ID3D12Resource> RenderBuffer() const { return mRenderTarget; }
	_WRL::ComPtr<ID3D12Resource> DepthBuffer() const { return mDepthBuffer; }
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle() const { return mRTVHeap->GetCPUDescriptorHandleForHeapStart(); };
	D3D12_CPU_DESCRIPTOR_HANDLE DSVHandle() const { return mDSVHeap->GetCPUDescriptorHandleForHeapStart(); };
	D3D12_GPU_DESCRIPTOR_HANDLE RTVGPUHandle() const { return mRTVHeap->GetGPUDescriptorHandleForHeapStart(); };
	D3D12_GPU_DESCRIPTOR_HANDLE DSVGPUHandle() const { return mDSVHeap->GetGPUDescriptorHandleForHeapStart(); };

	JAE_API void CreateRenderBuffers();
	JAE_API bool UpdateTransform();
	JAE_API void Clear(std::shared_ptr<CommandList> commandList);

private:
	friend class CommandList;

	void WriteCBuffer(unsigned int frameIndex);

	float mFieldOfView;
	float mNear;
	float mFar;
	unsigned int mPixelWidth;
	unsigned int mPixelHeight;
	unsigned int mMSAACount;

	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProjection;
	DirectX::XMFLOAT4X4 mViewProjection;
	DirectX::XMFLOAT4X4 mInverseProj;
	DirectX::XMFLOAT4 mPerspectiveBounds;

	DirectX::BoundingFrustum mFrustum;

	std::shared_ptr<ConstantBuffer> mCBuffer;

	DXGI_FORMAT mRenderFormat;
	DXGI_FORMAT mDepthFormat;
	_WRL::ComPtr<ID3D12Resource> mRenderTarget;
	_WRL::ComPtr<ID3D12Resource> mDepthBuffer;
	_WRL::ComPtr<ID3D12DescriptorHeap> mRTVHeap;
	_WRL::ComPtr<ID3D12DescriptorHeap> mDSVHeap;
};

