#pragma warning(disable : 3568) // unrecognized pragma
struct CameraBuffer {
	float4x4 View;
	float4x4 Projection;
	float4x4 ViewProjection;
	float4x4 InvProj;
	float4 Position;
	float4 Viewport;
};

#include "LightCommon.hlsli"

RWTexture2D<uint2> LightIndexBuffer : register(u0);
ConstantBuffer<CameraBuffer> Camera : register(b0);
ConstantBuffer<LightBuffer> Lighting : register(b1);

#pragma rootsig RootSig
#pragma compute main
#pragma Parameter srv LightIndexBuffer
#pragma Parameter cbuf CameraBuffer
#pragma Parameter cbuf LightBuffer

#define RootSig \
	"DescriptorTable(UAV(u0))," \
	"CBV(b0)," \
	"CBV(b1),"

bool Sphere(float3 ro, float3 rd, float3 pos, float radius) {
	float3 L = ro - pos;
	float a = dot(rd, rd);
	float b = 2 * dot(rd, L);
	float c = dot(L, L) - radius * radius;
	float det = b * b - 4 * a * c;
	return det > 0 && -b + sqrt(det) > 0;
}

[numthreads(8, 8, 1)]
void main(uint3 index : SV_DispatchThreadID) {
	float4 clip = float4((float2)index.xy / Lighting.LightIndexBufferSize.xy * 2 - 1, 0, 1);
	clip.y = -clip.y;
	float4 vp = mul(Camera.InvProj, clip);
	vp /= vp.w;
	float3 rd = normalize(vp.x * Camera.View[0].xyz + vp.y * Camera.View[1].xyz + vp.z * Camera.View[2].xyz);
	float3 ro = Camera.Position.xyz;

	uint2 v = 0;
	for (unsigned int i = 0; i < Lighting.LightCount; i++) {
		if (Lighting.Lights[i].Color.w == 0 || Sphere(ro, rd, Lighting.Lights[i].Position.xyz, Lighting.Lights[i].Position.w)) {
			if (i < 32)
				v.x |= 1 << i;
			else
				v.y |= 1 << (i - 32);
		}
	}
	LightIndexBuffer[index.xy] = v;
}