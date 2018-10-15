#include "Common.hlsli"

struct v2f {
	float4 pos : SV_Position;
	float3 worldPos : TEXCOORD0;
	float3 worldNormal : TEXCOORD1;
};

float4 main(v2f i) : SV_Target {
	float3 normal = normalize(i.worldNormal);

	return float4(normal * .5 + .5, 1.0);
}