struct Light {
	float4 Position; // position (xyz) range (w)
	float4 Color; // color-intensity premultiplied (rgb) intensity (a)
};
struct LightBuffer {
	Light Lights[64];
	uint LightCount;
	uint2 LightIndexBufferSize;
	uint pad;
	float4 GroundColor;
	float4 SkyColor;
};

Texture2D<uint2> LightIndexBuffer : register(t0);
ConstantBuffer<LightBuffer> Lighting : register(b2);

#pragma Parameter cbuf LightBuffer
#pragma Parameter srv LightIndexBuffer

#define RootSigLighting \
	"CBV(b2)," \
	"DescriptorTable(SRV(t0), visibility=SHADER_VISIBILITY_PIXEL),"

uint2 GetLightMask(float4 screenPos) {
	return LightIndexBuffer.Load(int3((screenPos.xy / Camera.Viewport.xy) * Lighting.LightIndexBufferSize.xy, 0)).xy;
}