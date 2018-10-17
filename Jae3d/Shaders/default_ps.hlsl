#include "Common.hlsli"

float4 main(v2f i) : SV_Target{
	float2 uv = float2(i.pack0.w, i.pack1.w);
	float3 worldPos = i.pack0.xyz;
	float3 normal = normalize(i.pack1.xyz);

	return float4(normal * .5 + .5, 1.0);
}