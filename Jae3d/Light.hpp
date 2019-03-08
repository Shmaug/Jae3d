#pragma once

#include "Object.hpp"
#include <DirectXCollision.h>
#include "Common.hpp"

class Light : public Object {
public:
	enum LIGHTMODE {
		LIGHTMODE_DIRECTIONAL,
		LIGHTMODE_SPOT,
		LIGHTMODE_POINT,
	};

	JAE_API Light(const jwstring& name);
	JAE_API ~Light();

	DirectX::XMFLOAT3 mColor;
	bool mShadows;
	float mShadowStrength;
	float mIntensity;
	float mRange;
	float mSpotAngle;
	LIGHTMODE mMode;

	JAE_API virtual DirectX::BoundingOrientedBox Bounds();
};

