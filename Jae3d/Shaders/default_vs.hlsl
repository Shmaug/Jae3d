#include "Common.hlsli"

struct appdata {
	float3 vertex : POSITION;
	float3 normal : NORMAL;
	float2 tex0 : TEXCOORD0;
};

v2f main(appdata v) {
	v2f o;

	float4 wp = mul(Object.ObjectToWorld, float4(v.vertex, 1));
	float3 wn = mul(float4(v.normal, 1), Object.WorldToObject).xyz;

	o.pos = mul(Camera.ViewProjection, wp);
	o.pack0 = float4(wp.xyz, v.tex0.x);
	o.pack1 = float4(wn.xyz, v.tex0.y);

	return o;
}