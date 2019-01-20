#pragma warning(disable : 3568) // unrecognized pragma
#pragma rootsig RootSig
#pragma vertex vsmain
#pragma pixel psmain

#define RootSig \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32bitconstants=20, b0)"

struct DataBuffer {
	float4x4 MVP;
	float4 Color;
};
ConstantBuffer<DataBuffer> Data : register(b0);

float4 vsmain(float3 vertex : POSITION) : SV_Position {
	return mul(Data.MVP, float4(vertex, 1));
}
float4 psmain(float4 pos : SV_Position) : SV_Target{
	return Data.Color;
}