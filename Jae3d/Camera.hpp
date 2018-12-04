#pragma once

#include <DirectXMath.h>
#include <wrl.h>

#include "Util.hpp"

#include "Object.hpp"

class ConstantBuffer;

class Camera : public Object {
protected:
	bool UpdateTransform();

public:
	Camera(jstring name);
	~Camera();

	float FieldOfView() const { return mFieldOfView; }
	float Aspect() const { return mAspect; }
	float Near() const { return mNear; }
	float Far() const { return mFar; }
	void FieldOfView(float f) { mFieldOfView = f; mTransformDirty = true; }
	void Aspect(float a) { mAspect = a; mTransformDirty = true; }
	void Near(float n) { mNear = n; mTransformDirty = true; }
	void Far(float f) { mFar = f; mTransformDirty = true; }

	DirectX::XMMATRIX View() { if (mTransformDirty) UpdateTransform(); return mView; }
	DirectX::XMMATRIX Projection() { if (mTransformDirty) UpdateTransform(); return mProjection; }
	DirectX::XMMATRIX ViewProjection() { if (mTransformDirty) UpdateTransform(); return mViewProjection; }

private:
	friend class CommandList;
	void SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	float mFieldOfView = 70.0f;
	float mAspect = 1.0f;
	float mNear = .01f;
	float mFar = 1000.0f;

	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProjection;
	DirectX::XMMATRIX mViewProjection;

	struct CameraBuffer {
	public:
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Projection;
		DirectX::XMFLOAT4X4 ViewProjection;
		DirectX::XMFLOAT3 CameraPosition;
	} mCameraBufferData;
	std::shared_ptr<ConstantBuffer> mCBuffer;
};

