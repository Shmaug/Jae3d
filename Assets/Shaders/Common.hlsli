struct CameraBuffer {
	float4x4 View;
	float4x4 Projection;
	float4x4 ViewProjection;
	float4 CameraPosition;
};
struct ObjectBuffer {
	float4x4 ObjectToWorld;
	float4x4 WorldToObject;
};
struct LightBuffer {
	float4 LightDir0;
	float4 LightCol0;
};

ConstantBuffer<ObjectBuffer> Object : register(b0);
ConstantBuffer<CameraBuffer> Camera : register(b1);
ConstantBuffer<LightBuffer> Light : register(b2);

#pragma Parameter cbuf CameraBuffer
#pragma Parameter cbuf ObjectBuffer
#pragma Parameter cbuf LightBuffer

#define RootSigCommon \
	"CBV(b0, space=0)," \
	"CBV(b1, space=0)," \
	"CBV(b2, space=0),"