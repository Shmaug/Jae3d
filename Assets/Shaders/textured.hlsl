#pragma warning(disable : 3568)
#pragma rootsig RootSig
#pragma vertex vsmain
#pragma pixel psmain

#include "Common.hlsli"

#pragma Parameter srv AlbedoRoughnessTex
#pragma Parameter srv NormalTex
#pragma Parameter srv MetallicTex

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
	float3 tangent : TANGENT;
	float2 tex0 : TEXCOORD0;
};
struct v2f {
	float4 pos : SV_Position;
	float4 tbn0 : TEXCOORD0;
	float4 tbn1 : TEXCOORD1;
	float4 tbn2 : TEXCOORD2;
	float2 uv : TEXCOORD3;
};

v2f vsmain(appdata v) {
	v2f o;

	float4 wp = mul(Object.ObjectToWorld, float4(v.vertex, 1));
	float3 wn = mul(float4(v.normal, 1), Object.WorldToObject).xyz;
	float3 wt = mul(float4(v.tangent, 1), Object.WorldToObject).xyz;
	float3 wb = cross(wn, wt);

	o.pos = mul(Camera.ViewProjection, wp);
	o.tbn0 = float4(wn.xyz, wp.x);
	o.tbn1 = float4(wt.xyz, wp.y);
	o.tbn2 = float4(wb.xyz, wp.z);
	o.uv = v.tex0.xy;

	return o;
}

float4 psmain(v2f i) : SV_Target{
	float3 LightCol = float3(1.0, 1.0, 1.0);
	float3 LightDir = normalize(float3(.25, -.5, -1.0));

	float3 normal = normalize(i.tbn0.xyz);
	float3 tangent = normalize(i.tbn1.xyz);
	float3 binormal = normalize(i.tbn2.xyz);
	float3 worldPos = float3(i.tbn0.w, i.tbn1.w, i.tbn2.w);
	float2 uv = i.uv.xy;

	float3 eye = normalize(worldPos - Camera.CameraPosition.xyz);

	float4 tex = AlbedoRoughnessTex.Sample(Sampler, uv);
	float3 bump = normalize(NormalTex.Sample(Sampler, uv).rgb * 2 - 1);
	float metal = MetallicTex.Sample(Sampler, uv).r;

	normal = tangent * bump.x + binormal * bump.y + normal * bump.z;

	float3 color = tex.rgb;
	float3 light = .025;
	float3 refl = 0.0;

	float ndotl = saturate(dot(normal, -LightDir));
	light += LightCol * ndotl;

	refl = LightCol * pow(saturate(dot(-reflect(LightDir, normal), eye)), lerp(tex.w * tex.w, 250, 100)) * metal;

	return float4(color * light + refl, 1.0);
}