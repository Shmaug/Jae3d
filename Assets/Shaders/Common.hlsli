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

ConstantBuffer<ObjectBuffer> Object : register(b0);
ConstantBuffer<CameraBuffer> Camera : register(b1);

#pragma Parameter cbv CameraBuffer
#pragma Parameter cbv ObjectBuffer

#define RootSigCommon \
	"CBV(b0, space = 0)," \
	"CBV(b1, space = 0),"