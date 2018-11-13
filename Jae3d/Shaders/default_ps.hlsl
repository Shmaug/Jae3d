#include "Common.hlsli"

float4 main(v2f i) : SV_Target {
	float3 LightCol = float3(1.0, 1.0, 1.0);
	float3 LightDir = normalize(float3(.25, -.5, -1.0));

	float2 uv = float2(i.pack0.w, i.pack1.w);
	float3 worldPos = i.pack0.xyz;
	float3 normal = normalize(i.pack1.xyz);
	float3 eye = normalize(worldPos - Camera.CameraPosition);

	float3 color = 1.0;
	float3 light = .025;
	float3 refl = 0.0;

	float ndotl = saturate(dot(normal, -LightDir));
	light += LightCol * ndotl;

	refl = LightCol * pow(saturate(dot(-reflect(LightDir, normal), eye)), 250);

	return float4(color * light + refl, 1.0);
}