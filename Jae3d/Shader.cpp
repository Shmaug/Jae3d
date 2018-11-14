#include "Shader.hpp"

#include <winbase.h>
#include <locale>
#include <codecvt>
#include <iostream>
#include <fstream>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "Graphics.hpp"
#include "Util.hpp"
#include "Mesh.hpp"
#include "RootSignature.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

map<string, Shader*> ShaderLibrary::shaders;

wstring s2ws(const std::string& str) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}
string ws2s(const std::wstring& wstr) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

void ReadIfExists(LPCWSTR file, ID3DBlob** blob) {
	string f = ws2s(wstring(file));
	OutputDebugString(("Loading shader " + f + "\n").c_str());
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(f.c_str()) && GetLastError() == ERROR_FILE_NOT_FOUND)
		return;
	ThrowIfFailed(D3DReadFileToBlob(file, blob));
}

Shader::Shader() {
	m_RootSignature = new RootSignature();
}
Shader::~Shader() {
	delete m_RootSignature;
}

void ShaderLibrary::LoadShader(ComPtr<ID3D12Device> device, string name) {
	Shader *s = new Shader();
	s->name = name;

	ReadIfExists(s2ws("Assets\\Shaders\\" + name + "_vs.cso").c_str(), &s->vertexBlob);
	ReadIfExists(s2ws("Assets\\Shaders\\" + name + "_ps.cso").c_str(), &s->pixelBlob);

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

	CD3DX12_ROOT_PARAMETER1 rootParams[2];
	rootParams[0].InitAsConstantBufferView(0, 0);
	rootParams[1].InitAsConstantBufferView(1, 0);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		  D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.Init_1_1(_countof(rootParams), rootParams, 0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> sigBlob;
	ComPtr<ID3DBlob> errBlob;
	ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSigDesc, sigBlob.GetAddressOf(), errBlob.GetAddressOf()));
	s->m_RootSignature->SetRootSignatureDesc(rootSigDesc.Desc_1_1, featureData.HighestVersion);

	DXGI_SAMPLE_DESC sampDesc = {};
	sampDesc.Count = Graphics::GetMSAASamples();
	sampDesc.Quality = 0;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	state.InputLayout = { Vertex::InputElements, Vertex::InputElementCount };
	state.pRootSignature = s->m_RootSignature->GetRootSignature().Get();
	state.VS = CD3DX12_SHADER_BYTECODE(s->vertexBlob.Get());
	state.PS = CD3DX12_SHADER_BYTECODE(s->pixelBlob.Get());
	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	state.SampleMask = UINT_MAX;
	state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	state.NumRenderTargets = 1;
	state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	state.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	state.SampleDesc = sampDesc;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&s->m_PipelineState)));

	shaders[name] = s;
}
void ShaderLibrary::LoadShaders() {
	auto device = Graphics::GetDevice();
	LoadShader(device, "default");
}
Shader* ShaderLibrary::GetShader(string name) {
	return shaders[name];
}