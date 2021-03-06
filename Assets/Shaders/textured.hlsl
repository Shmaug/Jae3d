#pragma warning(disable : 3568) // unrecognized pragma
#pragma rootsig RootSig
#pragma vertex vsmain
#pragma pixel psmain

#include <Core.hlsli>
#include "Lighting.hlsli"
#include "PBR.hlsli"

#pragma Parameter tbl MaterialTextures 3

#define RootSig \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |" \
	      "DENY_DOMAIN_SHADER_ROOT_ACCESS |" \
          "DENY_GEOMETRY_SHADER_ROOT_ACCESS |" \
          "DENY_HULL_SHADER_ROOT_ACCESS)," \
RootSigCore \
RootSigLighting \
RootSigPBR \
"DescriptorTable(SRV(t3), SRV(t4), SRV(t5), visibility=SHADER_VISIBILITY_PIXEL)"

Texture2D<float4> CombinedTex : register(t3);
Texture2D<float3> NormalTex   : register(t4);
Texture2D<float>  MetallicTex : register(t5);

struct v2f {
	float4 pos : SV_Position;
	float4 tbn0 : TEXCOORD0;
	float4 tbn1 : TEXCOORD1;
	float4 tbn2 : TEXCOORD2;
	float2 uv : TEXCOORD3;
};

struct appdata {
	float3 vertex : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 tex0 : TEXCOORD0;
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
	float3 normal = normalize(i.tbn0.xyz);
	float3 tangent = normalize(i.tbn1.xyz);
	float3 binormal = normalize(i.tbn2.xyz);
	float3 worldPos = float3(i.tbn0.w, i.tbn1.w, i.tbn2.w);
	float2 uv = i.uv.xy;

	float3 view = normalize(worldPos - Camera.Position.xyz);

	float4 tex = CombinedTex.Sample(Sampler, uv);

	float metallic = MetallicTex.Sample(Sampler, uv).r;

	Surface sfc;
	sfc.albedo = tex.rgb;
	sfc.metallic = metallic;
	sfc.smoothness = tex.a;
	float3 bump = normalize(NormalTex.Sample(Sampler, uv).rgb * 2 - 1);
	sfc.normal = normalize(tangent * bump.x + binormal * bump.y + normal * bump.z);
	sfc.worldPos = worldPos;

	float3 light = ShadePoint(sfc, i.pos, view);
	return float4(light, Material.alpha);
}