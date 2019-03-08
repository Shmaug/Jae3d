#pragma warning(disable : 3568) // unrecognized pragma
#pragma rootsig RootSig
#pragma vertex vsmain
#pragma pixel psmain

#include "Core.hlsli"

#pragma Parameter srv Texture

#define RootSig \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS )," \
RootSigCore \
"DescriptorTable(SRV(t0), visibility=SHADER_VISIBILITY_PIXEL)," \
"StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_LINEAR, visibility=SHADER_VISIBILITY_PIXEL)"

Texture2D<float4> Texture : register(t0);
sampler Sampler : register(s0); 

struct v2f {
	float4 pos : SV_Position;
	float2 tex : TEXCOORD0;
};

v2f vsmain(float3 vertex : POSITION, float4 tex : TEXCOORD0) {
	v2f o;
	float4 wp = mul(Object.ObjectToWorld, float4(vertex, 1));
	o.pos = mul(Camera.ViewProjection, wp);
	o.tex = tex.xy;
	return o;
}
float4 psmain(float4 pos : SV_Position, float2 tex : TEXCOORD0) : SV_Target {
	float4 c = Texture.Sample(Sampler, tex);
	clip(c.a - .01);
	return c;
}