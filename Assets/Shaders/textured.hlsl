#pragma warning(disable : 3568)
#pragma rootsig RootSig
#pragma vertex vsmain
#pragma pixel psmain

#include "Common.hlsli"

#pragma Parameter srv AlbedoRoughnessTex
#pragma Parameter srv NormalTex
#pragma Parameter srv MetallicTex
#pragma Parameter samp Sampler

#define RootSig \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |" \
	      "DENY_DOMAIN_SHADER_ROOT_ACCESS |" \
          "DENY_GEOMETRY_SHADER_ROOT_ACCESS |" \
          "DENY_HULL_SHADER_ROOT_ACCESS )," \
RootSigCommon \
"DescriptorTable(SRV(t0), visibility=SHADER_VISIBILITY_PIXEL)," \
"DescriptorTable(SRV(t1), visibility=SHADER_VISIBILITY_PIXEL)," \
"DescriptorTable(SRV(t2), visibility=SHADER_VISIBILITY_PIXEL)," \
"StaticSampler(s0, visibility=SHADER_VISIBILITY_PIXEL),"

Texture2D<float4> AlbedoRoughnessTex : register(t0);
Texture2D<float4> NormalTex : register(t1);
Texture2D<float4> MetallicTex : register(t2);
sampler Sampler : register(s0);

struct appdata {
	float3 vertex : POSITION;
	float3 normal : NORMAL;
	float2 tex0 : TEXCOORD0;
};
struct v2f {
	float4 pos : SV_Position;
	float4 pack0 : TEXCOORD0;
	float4 pack1 : TEXCOORD1;
};

v2f vsmain(appdata v) {
	v2f o;

	float4 wp = mul(Object.ObjectToWorld, float4(v.vertex, 1));
	float3 wn = mul(float4(v.normal, 1), Object.WorldToObject).xyz;

	o.pos = mul(Camera.ViewProjection, wp);
	o.pack0 = float4(wp.xyz, v.tex0.x);
	o.pack1 = float4(wn.xyz, v.tex0.y);

	return o;
}

float4 psmain(v2f i) : SV_Target{
	float3 LightCol = float3(1.0, 1.0, 1.0);
	float3 LightDir = normalize(float3(.25, -.5, -1.0));

	float2 uv = float2(i.pack0.w, i.pack1.w);
	float3 worldPos = i.pack0.xyz;
	float3 normal = normalize(i.pack1.xyz);
	float3 eye = normalize(worldPos - Camera.CameraPosition.xyz);

	float4 tex = AlbedoRoughnessTex.Sample(Sampler, uv);

	float3 color = tex.rgb;
	float3 light = .025;
	float3 refl = 0.0;

	float ndotl = saturate(dot(normal, -LightDir));
	light += LightCol * ndotl;

	refl = LightCol * pow(saturate(dot(-reflect(LightDir, normal), eye)), 250);

	return float4(color * light + refl, 1.0);
}