#include "Shader.hpp"

#include <d3d12.h>
#include <DirectXMath.h>

#include "Graphics.hpp"
#include "Util.hpp"
#include "Mesh.hpp"
#include "RootSignature.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

Shader::Shader(string name) : ShaderAsset(name) {}
Shader::Shader(string name, MemoryStream &ms) : ShaderAsset(name, ms) {}
Shader::~Shader() {
	m_States.clear();
}

bool Shader::SetActive(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!m_Created) Upload();
	commandList->SetGraphicsRootSignature(m_RootSignature.Get());
	return true;
}

void Shader::SetPSO(ComPtr<ID3D12GraphicsCommandList2> commandList, MeshAsset::SEMANTIC input) {
	if (m_States.count(input) == 0)
		m_States.emplace(input, CreatePSO(input));
	commandList->SetPipelineState(m_States[input].Get());
}
ComPtr<ID3D12PipelineState> Shader::CreatePSO(MeshAsset::SEMANTIC input){
	auto device = Graphics::GetDevice();

	UINT inputElementCount = 0;
	inputElementCount++; // position
	if (input & Mesh::SEMANTIC_NORMAL) inputElementCount++;
	if (input & Mesh::SEMANTIC_TANGENT) inputElementCount++;
	if (input & Mesh::SEMANTIC_BINORMAL) inputElementCount++;
	if (input & Mesh::SEMANTIC_COLOR0) inputElementCount++;
	if (input & Mesh::SEMANTIC_COLOR1) inputElementCount++;
	if (input & Mesh::SEMANTIC_BLENDINDICES) inputElementCount++;
	if (input & Mesh::SEMANTIC_BLENDWEIGHT) inputElementCount++;
	if (input & Mesh::SEMANTIC_TEXCOORD0) inputElementCount++;
	if (input & Mesh::SEMANTIC_TEXCOORD1) inputElementCount++;
	if (input & Mesh::SEMANTIC_TEXCOORD2) inputElementCount++;
	if (input & Mesh::SEMANTIC_TEXCOORD3) inputElementCount++;

	D3D12_INPUT_ELEMENT_DESC* inputElements = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
	int i = 0;
	inputElements[i++] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_NORMAL)
		inputElements[i++] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_TANGENT)
		inputElements[i++] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_BINORMAL)
		inputElements[i++] = { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_COLOR0)
		inputElements[i++] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_COLOR1)
		inputElements[i++] = { "COLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_BLENDINDICES)
		inputElements[i++] = { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_BLENDWEIGHT) 
		inputElements[i++] = { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_TEXCOORD0)
		inputElements[i++] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_TEXCOORD1)
		inputElements[i++] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_TEXCOORD2)
		inputElements[i++] = { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (input & Mesh::SEMANTIC_TEXCOORD3)
		inputElements[i++] = { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	DXGI_SAMPLE_DESC sampDesc = {};
	sampDesc.Count = Graphics::GetMSAASamples();
	sampDesc.Quality = 0;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	state.InputLayout = { inputElements, inputElementCount };
	state.pRootSignature = m_RootSignature.Get();// ->GetRootSignature().Get();
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

	ComPtr<ID3D12PipelineState> pso;
	ThrowIfFailed(device->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&pso)));

	delete[] inputElements;
	return pso;
}

void Shader::Upload() {
	if (m_Created) return;
	
	ID3DBlob* rsblob = GetBlob(SHADERSTAGE_ROOTSIG);
	if (rsblob)
		ThrowIfFailed(Graphics::GetDevice()->CreateRootSignature(1, rsblob->GetBufferPointer(), rsblob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
	
	m_Created = true;
}