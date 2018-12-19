#pragma once

#include "Object.hpp"
#include <DirectXCollision.h>
#include "Common.hpp"

class Light : public Object {
public:
	JAE_API Light(jwstring name);
	JAE_API ~Light();

	DirectX::XMFLOAT3 mColor;
	float mIntensity;
	float mRange;

	virtual DirectX::BoundingOrientedBox Bounds() { return DirectX::BoundingOrientedBox(WorldPosition(), DirectX::XMFLOAT3(mRange, mRange, mRange), WorldRotation()); }
};

