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
#include "AssetDatabase.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

Shader::Shader(string name) : ShaderAsset(name) {
	m_RootSignature = new RootSignature();
}
Shader::Shader(string name, MemoryStream &ms) : ShaderAsset(name, ms) {
	m_RootSignature = new RootSignature();
}
Shader::~Shader() {
	delete m_RootSignature;
}

bool Shader::SetActive(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!m_Created) Create();
	commandList->SetPipelineState(m_PipelineState.Get());
	commandList->SetGraphicsRootSignature(m_RootSignature->GetRootSignature().Get());
	return true;
}

void Shader::Create(){
	if (m_Created) return;
	m_Created = true;
	auto device = Graphics::GetDevice();

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
	m_RootSignature->SetRootSignatureDesc(rootSigDesc.Desc_1_1, featureData.HighestVersion);

	DXGI_SAMPLE_DESC sampDesc = {};
	sampDesc.Count = Graphics::GetMSAASamples();
	sampDesc.Quality = 0;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	state.InputLayout = { Vertex::InputElements, Vertex::InputElementCount };
	state.pRootSignature = m_RootSignature->GetRootSignature().Get();
	if (GetBlob(SHADERSTAGE_VERTEX))	state.VS = CD3DX12_SHADER_BYTECODE(GetBlob(SHADERSTAGE_VERTEX));
	if (GetBlob(SHADERSTAGE_HULL))		state.HS = CD3DX12_SHADER_BYTECODE(GetBlob(SHADERSTAGE_HULL));
	if (GetBlob(SHADERSTAGE_DOMAIN))	state.DS = CD3DX12_SHADER_BYTECODE(GetBlob(SHADERSTAGE_DOMAIN));
	if (GetBlob(SHADERSTAGE_GEOMETRY))	state.GS = CD3DX12_SHADER_BYTECODE(GetBlob(SHADERSTAGE_GEOMETRY));
	if (GetBlob(SHADERSTAGE_PIXEL))		state.PS = CD3DX12_SHADER_BYTECODE(GetBlob(SHADERSTAGE_PIXEL));
	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	state.SampleMask = UINT_MAX;
	state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	state.NumRenderTargets = 1;
	state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	state.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	state.SampleDesc = sampDesc;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&m_PipelineState)));

	if (GetBlob(SHADERSTAGE_VERTEX))	OutputDebugString("VERTEX  ");
	if (GetBlob(SHADERSTAGE_HULL))		OutputDebugString("HULL  ");
	if (GetBlob(SHADERSTAGE_DOMAIN))	OutputDebugString("DOMAIN  ");
	if (GetBlob(SHADERSTAGE_GEOMETRY))	OutputDebugString("GEOMETRY  ");
	if (GetBlob(SHADERSTAGE_PIXEL))		OutputDebugString("PIXEL  ");
	OutputDebugString("\n");
}