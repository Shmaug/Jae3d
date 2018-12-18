#pragma once

#include <DirectXMath.h>
#include <wrl.h>

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
protected:
	JAE_API bool UpdateTransform();

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

	void CalculateScreenLights(jvector<std::shared_ptr<Light>> &lights, unsigned int frameIndex);

	DirectX::XMMATRIX View() { if (mTransformDirty) UpdateTransform(); return mView; }
	DirectX::XMMATRIX Projection() { if (mTransformDirty) UpdateTransform(); return mProjection; }
	DirectX::XMMATRIX ViewProjection() { if (mTransformDirty) UpdateTransform(); return mViewProjection; }

private:
	friend class CommandList;
	void WriteCBuffer(unsigned int frameIndex);

	float mFieldOfView;
	float mNear;
	float mFar;
	unsigned int mPixelWidth;
	unsigned int mPixelHeight;

	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProjection;
	DirectX::XMMATRIX mViewProjection;
	DirectX::XMMATRIX mInverseView;
	DirectX::XMMATRIX mInverseProj;

	unsigned int mLightCount;
	std::shared_ptr<ConstantBuffer> mCBuffer;
	std::shared_ptr<Texture>* mLightIndexTexture;
	std::shared_ptr<ConstantBuffer> mLightBuffer;
};

