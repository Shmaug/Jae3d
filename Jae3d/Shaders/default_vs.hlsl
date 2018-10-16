#include "Common.hlsli"

struct appdata {
	float3 vertex : POSITION;
	float3 normal : NORMAL;
};

v2f main(appdata v) {
	v2f o;

	float4 wp = mul(Object.ObjectToWorld, float4(v.vertex, 1));

	o.pos = mul(Camera.ViewProjection, wp);
	o.worldPos = wp.xyz;
	o.worldNormal = mul(float4(v.normal, 1), Object.WorldToObject).xyz;

	return o;
}