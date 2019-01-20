#include "LightCommon.hlsli"

Texture2D<uint2> LightIndexBuffer : register(t0);
Texture2D<float> LightAtlas : register(t1);
ConstantBuffer<LightBuffer> Lighting : register(b2);

#pragma Parameter cbuf LightBuffer
#pragma Parameter tbl LightingTextures 2

#define RootSigLighting \
	"CBV(b2)," \
	"DescriptorTable(SRV(t0), SRV(t1), visibility=SHADER_VISIBILITY_PIXEL),"

uint2 GetLightMask(float4 screenPos) {
	return LightIndexBuffer.Load(int3((screenPos.xy / Camera.Viewport.xy) * Lighting.LightIndexBufferSize.xy, 0)).xy;
}

static const int3 o[9] = {
	int3(0, 0, 0),
	int3(-1,  0, 0),
	int3( 1,  0, 0),
	int3( 0,  1, 0),
	int3( 0, -1, 0),
	int3(-1, -1, 0),
	int3( 1, -1, 0),
	int3(-1,  1, 0),
	int3(-1,  1, 0)
};

float SampleShadowPCF(float2 uv, float z, ShadowLight s) {
	int3 p = int3(s.TexelSize.xy + uv * s.TexelSize.zw, 0);
	float avg = 0;
	for (uint i = 0; i < 9; i++)
		avg += z < LightAtlas.Load(p + o[i]).r;
	return avg * .11111111;
}

float ShadowAttenuation(float3 worldPos, float3 normal, Light light) {
	float a = 1;
	if (light.ShadowIndex != -1) {
		ShadowLight s = Lighting.Shadows[light.ShadowIndex];
		float z = mul(s.View, float4(worldPos + normal * s.NormalBias, 1)).z + s.Bias;
		float2 uv = mul(s.ViewProjection, float4(worldPos, 1)).xy * .5 + .5;
		uv.y = 1.0 - uv.y;
		if (z > 0 && uv.x > 0 && uv.y > 0 && uv.x < 1 && uv.y < 1)
			a = lerp(1, SampleShadowPCF(uv, z, s), s.Strength);
	}
	return a;
}