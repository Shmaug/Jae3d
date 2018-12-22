#pragma warning(disable : 3568) // unrecognized pragma
#pragma rootsig RootSig
#pragma vertex vsmain
#pragma pixel psmain

#define RootSig \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS )," \
"RootConstants(num32BitConstants=16, b0, visibility=SHADER_VISIBILITY_VERTEX)"

struct CB { float4x4 proj; };
ConstantBuffer<CB> C : register(b0);

struct v2f {
	float4 pos : SV_Position;
	float4 color : COLOR0;
};

v2f vsmain(float3 vertex : POSITION, float4 color : COLOR0) {
	v2f o;
	o.pos = mul(C.proj, float4(vertex, 1));
	o.color = color;
	return o;
}
float4 psmain(float4 pos : SV_Position, float4 color : COLOR0) : SV_Target{
	return color;
}