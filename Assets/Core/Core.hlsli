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

ConstantBuffer<ObjectBuffer> Object : register(b0);
ConstantBuffer<CameraBuffer> Camera : register(b1);

#pragma Parameter cbuf ObjectBuffer
#pragma Parameter cbuf CameraBuffer

#define RootSigCore  "CBV(b0), CBV(b1),"