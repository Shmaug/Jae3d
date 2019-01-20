#pragma once

#include "Asset.hpp"
#include "Common.hpp"

class Animation : public Asset {
public:
	JAE_API Animation(jwstring name);
	JAE_API Animation(jwstring name, MemoryStream &ms);
	JAE_API ~Animation();

	JAE_API virtual void WriteData(MemoryStream &ms) override;
	JAE_API virtual uint64_t TypeId() override;
};

