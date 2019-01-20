#pragma warning(disable : 3568) // unrecognized pragma
#pragma rootsig RootSig
#pragma vertex vsmain
#pragma pixel psmain

#include <Core.hlsli>

#pragma Parameter srv Texture

#define RootSig \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |" \
	      "DENY_DOMAIN_SHADER_ROOT_ACCESS |" \
          "DENY_GEOMETRY_SHADER_ROOT_ACCESS |" \
          "DENY_HULL_SHADER_ROOT_ACCESS )," \
RootSigCore \
"DescriptorTable(SRV(t1), visibility=SHADER_VISIBILITY_PIXEL)," \
"StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_LINEAR, visibility=SHADER_VISIBILITY_PIXEL)"

TextureCube<float3> Sky : register(t1);
sampler Sampler : register(s0);

struct v2f {
	float4 pos : SV_Position;
	float3 ray : TEXCOORD0;
};

v2f vsmain(float3 vertex : POSITION) {
	v2f o;

	o.pos = mul(Camera.ViewProjection, float4(vertex + Camera.Position.xyz, 1));
	o.ray = vertex;

	return o;
}

float4 psmain(v2f i) : SV_Target{
	float3 ro = Camera.Position.xyz;
	float3 rd = normalize(i.ray);
	return float4(Sky.Sample(Sampler, rd), 1);
}