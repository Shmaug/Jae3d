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


Camera::Camera(jwstring name) : Object(name), mFieldOfView(70.0f), mNear(.1f), mFar(1000.0f) {
	mPixelWidth = Graphics::GetWindow()->GetWidth();
	mPixelHeight = Graphics::GetWindow()->GetHeight();

	size_t size = sizeof(XMFLOAT4X4) * 4 + sizeof(XMFLOAT4) * 2;
	auto cb = new ConstantBuffer(size, L"Camera CB", Graphics::BufferCount());
	mCBuffer = shared_ptr<ConstantBuffer>(cb);
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

bool Camera::UpdateTransform(){
	if (!Object::UpdateTransform()) return false;

	XMVECTOR worldPos = XMLoadFloat3(&WorldPosition());
	XMVECTOR worldRot = XMLoadFloat4(&WorldRotation());

	XMMATRIX view = XMMatrixLookToLH(worldPos, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), worldRot), XMVector3Rotate(XMVectorSet(0, 1, 0, 0), worldRot));
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(mFieldOfView), (float)mPixelWidth / (float)mPixelHeight , mNear, mFar);

	XMStoreFloat4x4(&mView, view);
	XMStoreFloat4x4(&mProjection, proj);
	XMStoreFloat4x4(&mViewProjection, view * proj);
	XMStoreFloat4x4(&mInverseProj, XMMatrixInverse(&XMMatrixDeterminant(proj), proj));

	BoundingFrustum::CreateFromMatrix(mFrustum, proj);
	mFrustum.Orientation = WorldRotation();
	mFrustum.Origin = WorldPosition();

	return true;
}