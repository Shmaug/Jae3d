#pragma warning(disable : 3568) // unrecognized pragma
#pragma rootsig RootSig
#pragma vertex vsmain
#pragma pixel psmain

#include "Core.hlsli"

#define RootSig \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
RootSigCore \
"RootConstants(num32bitconstants=4, b2)"

struct MaterialBuffer {
	float4 Color;
};
ConstantBuffer<MaterialBuffer> Material : register(b2);

float4 vsmain(float3 vertex : POSITION) : SV_Position {
	return mul(Camera.ViewProjection, mul(Object.ObjectToWorld, float4(vertex, 1)));
}
float4 psmain(float4 pos : SV_Position) : SV_Target{
	return Material.Color;
}