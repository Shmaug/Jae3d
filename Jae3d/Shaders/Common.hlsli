struct CameraBuffer {
	float4x4 ViewProjection;
	float4x4 View;
	float4x4 Projection;
	float4 CameraPosition;
};
struct ObjectBuffer {
	float4x4 ObjectToWorld;
	float4x4 WorldToObject;
};

ConstantBuffer<CameraBuffer> Camera : register(b0);
ConstantBuffer<ObjectBuffer> Object : register(b1);

struct v2f {
	float4 pos : SV_Position;
	float3 worldPos : TEXCOORD0;
	float3 worldNormal : TEXCOORD1;
};