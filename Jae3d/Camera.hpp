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

	inline DirectX::XMFLOAT4 PerspectiveBounds() const { return mPerspectiveBounds; }
	inline float FieldOfView() const { return mFieldOfView; }
	inline float Near() const { return mNear; }
	inline float Far() const { return mFar; }
	inline unsigned int PixelWidth() const { return mPixelWidth; }
	inline unsigned int PixelHeight() const { return mPixelHeight; }

	inline void PerspectiveBounds(DirectX::XMFLOAT4 f) { mPerspectiveBounds = f; mFieldOfView = 0; mTransformDirty = true; }
	inline void FieldOfView(float f) { mPerspectiveBounds = { 0,0,0,0 }; mFieldOfView = f; mTransformDirty = true; }
	inline void Near(float n) { mNear = n; mTransformDirty = true; }
	inline void Far(float f) { mFar = f; mTransformDirty = true; }
	inline void PixelWidth(unsigned int w) { mPixelWidth = w; mTransformDirty = true; }
	inline void PixelHeight(unsigned int h) { mPixelHeight = h; mTransformDirty = true; }
	 
	inline unsigned int SampleCount() { return mSampleCount; }
	inline void SampleCount(unsigned int s) { mSampleCount = s; }

	DirectX::XMFLOAT4X4 View() { UpdateTransform(); return mView; }
	DirectX::XMFLOAT4X4 Projection() { UpdateTransform(); return mProjection; }
	DirectX::XMFLOAT4X4 ViewProjection() { UpdateTransform(); return mViewProjection; }
	DirectX::BoundingFrustum Frustum() { UpdateTransform(); return mFrustum; }
	std::shared_ptr<ConstantBuffer> CBuffer() { return mCBuffer; }

	inline bool Orthographic() const { return mOrthographic; }
	inline void Orthographic(bool o) { mOrthographic = o; }
	inline float OrthographicSize() const { return mOrthographicSize; }
	inline void OrthographicSize(float o) { mOrthographicSize = o; }
	inline void RenderFormat(DXGI_FORMAT f) { mRenderFormat = f; }
	inline DXGI_FORMAT RenderFormat() const { return mRenderFormat; }
	inline void DepthFormat(DXGI_FORMAT f) { mDepthFormat = f; }
	inline DXGI_FORMAT DepthFormat() const { return mDepthFormat; }
	inline _WRL::ComPtr<ID3D12Resource> RenderBuffer() const { return mRenderTarget; }
	inline _WRL::ComPtr<ID3D12Resource> DepthBuffer() const { return mDepthBuffer; }
	inline _WRL::ComPtr<ID3D12DescriptorHeap> RTVHeap() const { return mRTVHeap; }
	inline _WRL::ComPtr<ID3D12DescriptorHeap> DSVHeap() const { return mDSVHeap; }
	inline D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle() const { return mRTVHeap->GetCPUDescriptorHandleForHeapStart(); };
	inline D3D12_CPU_DESCRIPTOR_HANDLE DSVHandle() const { return mDSVHeap->GetCPUDescriptorHandleForHeapStart(); };
	inline D3D12_GPU_DESCRIPTOR_HANDLE RTVGPUHandle() const { return mRTVHeap->GetGPUDescriptorHandleForHeapStart(); };
	inline D3D12_GPU_DESCRIPTOR_HANDLE DSVGPUHandle() const { return mDSVHeap->GetGPUDescriptorHandleForHeapStart(); };

	JAE_API void CreateRenderBuffers();
	JAE_API bool UpdateTransform() override;
	JAE_API void Clear(std::shared_ptr<CommandList> commandList, DirectX::XMFLOAT4 color = { 0.0f, 0.0f, 0.0f, 1.f });

private:
	friend class CommandList;

	void WriteCBuffer(unsigned int frameIndex);

	bool mOrthographic;
	float mOrthographicSize;
	float mFieldOfView;
	float mNear;
	float mFar;
	unsigned int mPixelWidth;
	unsigned int mPixelHeight;
	unsigned int mSampleCount;

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

