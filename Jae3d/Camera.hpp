#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>

#include "Util.hpp"
#include <d3d12.h>
#include "d3dx12.hpp"

#include "Object.hpp"

class ConstantBuffer;
class Texture;
class Shader;
class Scene;
class Light;

class Camera : public Object {
public:
	JAE_API Camera(jwstring name);
	JAE_API ~Camera();

	float FieldOfView() const { return mFieldOfView; }
	float Near() const { return mNear; }
	float Far() const { return mFar; }
	unsigned int PixelWidth() const { return mPixelWidth; }
	unsigned int PixelHeight() const { return mPixelHeight; }
	void FieldOfView(float f) { mFieldOfView = f; mTransformDirty = true; }
	void Near(float n) { mNear = n; mTransformDirty = true; }
	void Far(float f) { mFar = f; mTransformDirty = true; }
	void PixelWidth(unsigned int w) { mPixelWidth = w; mTransformDirty = true; }
	void PixelHeight(unsigned int h) { mPixelHeight = h; mTransformDirty = true; }

	void CalculateScreenLights(Scene* scene, unsigned int frameIndex);

	DirectX::XMFLOAT4X4 View() { UpdateTransform(); return mView; }
	DirectX::XMFLOAT4X4 Projection() { UpdateTransform(); return mProjection; }
	DirectX::XMFLOAT4X4 ViewProjection() { UpdateTransform(); return mViewProjection; }
	DirectX::BoundingFrustum Frustum() { UpdateTransform(); return mFrustum; }

	JAE_API bool UpdateTransform();

private:
	friend class CommandList;

	void WriteCBuffer(unsigned int frameIndex);

	float mFieldOfView;
	float mNear;
	float mFar;
	unsigned int mPixelWidth;
	unsigned int mPixelHeight;

	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProjection;
	DirectX::XMFLOAT4X4 mViewProjection;
	DirectX::XMFLOAT4X4 mInverseProj;

	DirectX::BoundingFrustum mFrustum;

	unsigned int mLightCount;
	std::shared_ptr<ConstantBuffer> mCBuffer;
	std::shared_ptr<Texture>* mLightIndexTexture;
	std::shared_ptr<ConstantBuffer> mLightBuffer;
};

