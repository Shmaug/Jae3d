#pragma warning(disable : 3568)
#pragma rootsig RootSig
#pragma vertex vsmain
#pragma pixel psmain

#include "Common.hlsli"

#define RootSig \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |" \
	      "DENY_DOMAIN_SHADER_ROOT_ACCESS |" \
          "DENY_GEOMETRY_SHADER_ROOT_ACCESS |" \
          "DENY_HULL_SHADER_ROOT_ACCESS )," \
RootSigCommon

struct appdata {
	float3 vertex : POSITION;
	float3 normal : NORMAL;
};
struct v2f {
	float4 pos : SV_Position;
	float3 normal : NORMAL;
};

v2f vsmain(appdata v) {
	v2f o;

	float4 wp = mul(Object.ObjectToWorld, float4(v.vertex, 1));
	float3 wn = mul(float4(v.normal, 1), Object.WorldToObject).xyz;

	o.pos = mul(Camera.ViewProjection, wp);
	o.normal = wn;

	return o;
}

float4 psmain(v2f i) : SV_Target{
	float3 LightCol = float3(1.0, 1.0, 1.0);
	float3 LightDir = normalize(float3(.25, -.5, -1.0));

	float3 color = 1.0;
	color *= .025 + LightCol * saturate(dot(normalize(i.normal.xyz), -LightDir));

	return float4(color, 1.0);
}