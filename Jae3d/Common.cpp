#include "Common.hpp"

using namespace std;

MaterialValue::MaterialValue() : cbufferIndex(-1), tableSize(-1),
floatValue(0), float2Value({ 0,0 }), float3Value({ 0,0,0 }), float4Value({ 0,0,0,0 }),
intValue(0), int2Value({ 0,0 }), int3Value({ 0,0,0 }), int4Value({ 0,0,0,0 }),
uintValue(0), uint2Value({ 0,0 }), uint3Value({ 0,0,0 }), uint4Value({ 0,0,0,0 }),
textureValue(nullptr), cbufferValue(nullptr), tableValue(nullptr) {
	range.floatRange = DirectX::XMFLOAT2();
}

MaterialValue::MaterialValue(const MaterialValue &mv) : cbufferIndex(mv.cbufferIndex), tableSize(mv.tableSize) {
	memcpy(&range, &mv.range, sizeof(Range));
	floatValue = mv.floatValue;
	float2Value = mv.float2Value;
	float3Value = mv.float3Value;
	float4Value = mv.float4Value;

	intValue = mv.intValue;
	int2Value = mv.int2Value;
	int3Value = mv.int3Value;
	int4Value = mv.int4Value;

	uintValue = mv.uintValue;
	uint2Value = mv.uint2Value;
	uint3Value = mv.uint3Value;
	uint4Value = mv.uint4Value;

	textureValue = mv.textureValue;
	cbufferValue = mv.cbufferValue;
	tableValue = mv.tableValue;
}

void MaterialValue::set(float val) {
	floatValue = val;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(DirectX::XMFLOAT2 val) {
	floatValue = 0;
	float2Value = val;
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(DirectX::XMFLOAT3 val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = val;
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(DirectX::XMFLOAT4 val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = val;

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(int val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = val;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(DirectX::XMINT2 val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = val;
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(DirectX::XMINT3 val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = val;
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(DirectX::XMINT4 val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = val;

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(unsigned int val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = val;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(DirectX::XMUINT2 val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = val;
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(DirectX::XMUINT3 val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = val;
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(DirectX::XMUINT4 val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = val;

	textureValue.reset();
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(const std::shared_ptr<Texture>& val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue = val;
	cbufferValue.reset();
	tableValue.reset();
}
void MaterialValue::set(const std::shared_ptr<ConstantBuffer>& val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue = val;
	tableValue.reset();
}
void MaterialValue::set(const std::shared_ptr<DescriptorTable>& val) {
	floatValue = 0;
	float2Value = { 0,0 };
	float3Value = { 0,0,0 };
	float4Value = { 0,0,0,0 };

	intValue = 0;
	int2Value = { 0,0 };
	int3Value = { 0,0,0 };
	int4Value = { 0,0,0,0 };

	uintValue = 0;
	uint2Value = { 0,0 };
	uint3Value = { 0,0,0 };
	uint4Value = { 0,0,0,0 };

	textureValue.reset();
	cbufferValue.reset();
	tableValue = val;
}

MaterialValue& MaterialValue::operator=(const MaterialValue &rhs) {
	if (&rhs == this) return *this;
	memcpy(&range, &rhs.range, sizeof(Range));
	floatValue = rhs.floatValue;
	float2Value = rhs.float2Value;
	float3Value = rhs.float3Value;
	float4Value = rhs.float4Value;

	intValue = rhs.intValue;
	int2Value = rhs.int2Value;
	int3Value = rhs.int3Value;
	int4Value = rhs.int4Value;

	uintValue = rhs.uintValue;
	uint2Value = rhs.uint2Value;
	uint3Value = rhs.uint3Value;
	uint4Value = rhs.uint4Value;

	textureValue = rhs.textureValue;
	cbufferValue = rhs.cbufferValue;
	tableValue = rhs.tableValue;
	cbufferIndex = rhs.cbufferIndex;
	tableSize = rhs.tableSize;
	return *this;
}