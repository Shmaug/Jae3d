#include "Light.hpp"

using namespace DirectX;

Light::Light(const jwstring& name)
	: Object(name), mColor({ 1.0f, 1.0f, 1.0f }), mShadows(false), mShadowStrength(1.0f), mIntensity(1.0f), mRange(5.0f), mSpotAngle(.2f), mMode(LIGHTMODE_POINT) {}
Light::~Light() {}

BoundingOrientedBox Light::Bounds() {
	switch (mMode) {
	case LIGHTMODE_DIRECTIONAL:
		return DirectX::BoundingOrientedBox({ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0, 1 });
	default:
	case LIGHTMODE_POINT:
		return DirectX::BoundingOrientedBox(WorldPosition(), { mRange, mRange, mRange }, { 0, 0, 0, 1 });
	case LIGHTMODE_SPOT:
	{
		float r = tanf(mSpotAngle * .5f) * mRange;
		XMVECTOR fwd = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&WorldRotation()));
		XMFLOAT3 p;
		XMStoreFloat3(&p, XMVectorMultiplyAdd(fwd, XMVectorSet(mRange * .5f, mRange * .5f, mRange * .5f, 1.0f), XMLoadFloat3(&WorldPosition())));
		return BoundingOrientedBox(p, { r * .5f, r * .5f, mRange * .5f }, WorldRotation());
	}
	}
}