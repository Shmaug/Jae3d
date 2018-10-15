struct CameraData {
	float4x4 ViewProjection;
	float4x4 View;
	float4x4 Projection;
	float4 CameraPosition;

	float4x4 ObjectToWorld;
	float4x4 WorldToObject;
};

ConstantBuffer<CameraData> Camera : register(b0);