#pragma once

#include "Object.hpp"
#include "Common.hpp"

class Light : public Object {
public:
	JAE_API Light(jwstring name);
	JAE_API ~Light();

	DirectX::XMVECTOR mColor;
	float mIntensity;
	float mRange;
};

