struct CameraBuffer {
	float4x4 View;
	float4x4 Projection;
	float4x4 ViewProjection;
	float4x4 InvProj;
	float4 Position;
	float4 Viewport;
};
struct ObjectBuffer {
	float4x4 ObjectToWorld;
	float4x4 WorldToObject;
};

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

ConstantBuffer<ObjectBuffer> Object : register(b0);
ConstantBuffer<CameraBuffer> Camera : register(b1);

Texture2D<uint2> LightIndexBuffer : register(t0);
ConstantBuffer<LightBuffer> Lighting : register(b2);

#pragma Parameter cbuf LightBuffer
#pragma Parameter srv LightIndexBuffer
#pragma Parameter cbuf ObjectBuffer
#pragma Parameter cbuf CameraBuffer

#define RootSigCommon \
	"CBV(b2)," \
	"DescriptorTable(SRV(t0), visibility=SHADER_VISIBILITY_PIXEL)," \
	"CBV(b0)," \
	"CBV(b1),"

uint2 GetLightMask(float4 screenPos) {
	return LightIndexBuffer.Load(int3((screenPos.xy / Camera.Viewport.xy) * Lighting.LightIndexBufferSize.xy, 0)).xy;
}