#include "Camera.hpp"

#include <DirectXMath.h>
#include "Graphics.hpp"
#include "ConstantBuffer.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "CommandQueue.hpp"
#include "CommandList.hpp"
#include "AssetDatabase.hpp"
#include "Profiler.hpp"
#include "Light.hpp"
#include "Scene.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

Camera::Camera(const jwstring& name)
	: Object(name), mFieldOfView(70.0f), mNear(.1f), mFar(1000.0f), mOrthographicSize(2.0f), mOrthographic(false),
	mDepthFormat(DXGI_FORMAT_D32_FLOAT), mRenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM), mSampleCount(4) {

	mPixelWidth = Graphics::GetWindow()->GetWidth();
	mPixelHeight = Graphics::GetWindow()->GetHeight();

	size_t size = sizeof(XMFLOAT4X4) * 4 + sizeof(XMFLOAT4) * 2;
	mCBuffer = shared_ptr<ConstantBuffer>(new ConstantBuffer(size, mName + L" CB", Graphics::BufferCount()));

	mDSVHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
	mDSVHeap->SetName((mName + L" DSV Heap").c_str());
	mRTVHeap = Graphics::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
	mDSVHeap->SetName((mName + L" RTV Heap").c_str());
	CreateRenderBuffers();
}
Camera::~Camera() {}

void Camera::WriteCBuffer(unsigned int frameIndex){
	UpdateTransform();

	unsigned int o = 0;
	mCBuffer->WriteFloat4x4(mView, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat4x4(mProjection, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat4x4(mViewProjection, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat4x4(mInverseProj, o, frameIndex); o += sizeof(XMFLOAT4X4);
	mCBuffer->WriteFloat3(WorldPosition(), o, frameIndex); o += sizeof(XMFLOAT4);
	mCBuffer->WriteFloat4(XMFLOAT4((float)mPixelWidth, (float)mPixelHeight, mNear, mFar), o, frameIndex);
}

void Camera::CreateRenderBuffers(){
	auto device = Graphics::GetDevice();

#pragma region render buffer
	D3D12_RESOURCE_DESC rtvdesc = CD3DX12_RESOURCE_DESC::Tex2D(
		mRenderFormat,
		mPixelWidth,
		mPixelHeight,
		1, // This render target view has only one texture.
		1, // Use a single mipmap level
		mSampleCount);
	rtvdesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE rtvClearValue = {};
	rtvClearValue.Format = mRenderFormat;
	rtvClearValue.DepthStencil = { 1.0f, 0 };
	rtvClearValue.Color[0] = 0.0f;
	rtvClearValue.Color[1] = 0.0f;
	rtvClearValue.Color[2] = 0.0f;
	rtvClearValue.Color[3] = 1.0f;

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&rtvdesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&rtvClearValue,
		IID_PPV_ARGS(mRenderTarget.ReleaseAndGetAddressOf())
	));
	mRenderTarget->SetName((mName + L" Render Target").c_str());

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = mRenderFormat;
	if (mSampleCount > 0)
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
	else
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(mRenderTarget.Get(), &rtvDesc, mRTVHeap->GetCPUDescriptorHandleForHeapStart());
#pragma endregion
#pragma region depth buffer
	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = mDepthFormat;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(mDepthFormat, mPixelWidth, mPixelHeight, 1, 1, mSampleCount, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optimizedClearValue,
		IID_PPV_ARGS(mDepthBuffer.ReleaseAndGetAddressOf())
	));
	mDepthBuffer->SetName((mName + L" Depth Buffer").c_str());

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
	dsv.Format = mDepthFormat;
	if (mSampleCount > 0)
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	else
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv.Texture2D.MipSlice = 0;
	dsv.Flags = D3D12_DSV_FLAG_NONE;
	device->CreateDepthStencilView(mDepthBuffer.Get(), &dsv, mDSVHeap->GetCPUDescriptorHandleForHeapStart());
#pragma endregion
}

void Camera::Clear(const shared_ptr<CommandList>& commandList, const XMFLOAT4& color) {
	commandList->D3DCommandList()->ClearRenderTargetView(RTVHandle(), (float*)&color, 0, nullptr);
	commandList->D3DCommandList()->ClearDepthStencilView(DSVHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

bool Camera::UpdateTransform(){
	if (!Object::UpdateTransform()) return false;

	XMVECTOR worldPos = XMLoadFloat3(&WorldPosition());
	XMVECTOR worldRot = XMLoadFloat4(&WorldRotation());

	XMMATRIX view = XMMatrixLookToLH(worldPos, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), worldRot), XMVector3Rotate(XMVectorSet(0, 1, 0, 0), worldRot));
	XMMATRIX proj;

	if (mOrthographic) {
		proj = XMMatrixOrthographicLH(mOrthographicSize, (float)mOrthographicSize * mPixelHeight / (float)mPixelWidth, mNear, mFar);
	} else {
		if (mFieldOfView)
			proj = XMMatrixPerspectiveFovLH(mFieldOfView, (float)mPixelWidth / (float)mPixelHeight, mNear, mFar);
		else
			proj = XMMatrixPerspectiveOffCenterLH(mPerspectiveBounds.x, mPerspectiveBounds.y, mPerspectiveBounds.z, mPerspectiveBounds.w, mNear, mFar);
	}
	XMStoreFloat4x4(&mView, view);
	XMStoreFloat4x4(&mProjection, proj);
	XMStoreFloat4x4(&mViewProjection, XMMatrixMultiply(view, proj));
	XMStoreFloat4x4(&mInverseProj, XMMatrixInverse(&XMMatrixDeterminant(proj), proj));

	BoundingFrustum::CreateFromMatrix(mFrustum, proj);
	mFrustum.Orientation = WorldRotation();
	mFrustum.Origin = WorldPosition();

	return true;
}