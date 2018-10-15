#include "Common.hlsli"

struct appdata {
	float3 vertex : POSITION;
	float3 normal : NORMAL;
};

struct v2f {
	float4 pos : SV_Position;
	float3 worldPos : TEXCOORD0;
	float3 worldNormal : TEXCOORD1;
};

v2f main(appdata v) {
	v2f o;

	float4 wp = mul(Camera.ObjectToWorld, float4(v.vertex, 1));

	o.pos = mul(Camera.ViewProjection, wp);
	o.worldPos = wp.xyz;
	o.worldNormal = mul(float4(v.normal, 1), Camera.WorldToObject).xyz;

	return o;
}