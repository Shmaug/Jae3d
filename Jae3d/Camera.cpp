#include "Camera.hpp"

#include <DirectXMath.h>
#include "Graphics.hpp"
#include "ConstantBuffer.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

Camera::Camera(jwstring name) : Object(name) {
	mCBuffer = shared_ptr<ConstantBuffer>(new ConstantBuffer(sizeof(CameraBuffer), L"Camera CB", Graphics::BufferCount()));
}
Camera::~Camera() {}

void Camera::SetActive(ComPtr<ID3D12GraphicsCommandList2> commandList, unsigned int frameIndex){
	XMStoreFloat4x4(&mCameraBufferData.View, mView);
	XMStoreFloat4x4(&mCameraBufferData.Projection, mProjection);
	XMStoreFloat4x4(&mCameraBufferData.ViewProjection, mViewProjection);
	XMStoreFloat3(&mCameraBufferData.CameraPosition, WorldPosition());
	mCBuffer->Write(&mCameraBufferData, sizeof(CameraBuffer), 0, frameIndex);

	commandList->SetGraphicsRootConstantBufferView(1, mCBuffer->GetGPUAddress(frameIndex));
}

bool Camera::UpdateTransform(){
	if (!Object::UpdateTransform()) return false;

	XMVECTOR fwd = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), WorldRotation());
	XMVECTOR up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), WorldRotation());
	mView = XMMatrixLookToLH(WorldPosition(), fwd, up);
	mProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(mFieldOfView), mAspect, mNear, mFar);
	mViewProjection = mView * mProjection;

	return true;
}