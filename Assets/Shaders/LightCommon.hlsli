struct Light {
	float4 Position; // position (xyz) range (w)
	float4 Color; // color-intensity premultiplied (rgb) mode (a)
	float4 Direction; // direction (xyz) angle (w)
	int ShadowIndex;
	uint3 pad;
};
struct ShadowLight {
	uint4 TexelSize; // position (xy) size (zw)
	float4x4 View;
	float4x4 ViewProjection;
	float Strength;
	float Bias;
	float NormalBias;
	float pad;
};
struct LightBuffer {
	Light Lights[64];
	ShadowLight Shadows[64];
	uint LightCount;
	uint2 LightIndexBufferSize;
	uint pad;
	float4 GroundColor;
	float4 SkyColor;
};