#include "Common.hlsli"

float4 main(v2f i) : SV_Target {
	float3 normal = normalize(i.worldNormal);
	return float4(normal * .5 + .5, 1.0);
}