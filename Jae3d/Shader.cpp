#include "Shader.hpp"

#include <d3d12.h>
#include <DirectXMath.h>

#include "Graphics.hpp"
#include "Mesh.hpp"
#include "Window.hpp"

using namespace DirectX;
using namespace Microsoft::WRL;

Shader::Shader(jstring name) : ShaderAsset(name) {}
Shader::Shader(jstring name, MemoryStream &ms) : ShaderAsset(name, ms) {}
Shader::~Shader() {
	mStates.clear();
}

// Set the root signature on the GPU
bool Shader::SetActive(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!mCreated) Upload();
	if (mRootSignature) {
		commandList->SetGraphicsRootSignature(mRootSignature.Get());
		return true;
	}
	return false;
}

void Shader::SetPSO(ComPtr<ID3D12GraphicsCommandList2> commandList, MeshAsset::SEMANTIC input) {
	if (!mStates.has(input))
		mStates.emplace(input, CreatePSO(input));
	commandList->SetPipelineState(mStates.at(input).Get());
}

void Shader::SetCompute(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!mCreated) Upload();

	if (mRootSignature) {
		commandList->SetComputeRootSignature(mRootSignature.Get());

		if (!mComputePSO) CreateComputePSO();
		commandList->SetPipelineState(mComputePSO.Get());
	}
}

ComPtr<ID3D12PipelineState> Shader::CreatePSO(MeshAsset::SEMANTIC input) {
	auto device = Graphics::GetDevice();

	if (!GetBlob(SHADERSTAGE_VERTEX) && !GetBlob(SHADERSTAGE_HULL) &&
		!GetBlob(SHADERSTAGE_DOMAIN) && !GetBlob(SHADERSTAGE_GEOMETRY) && !GetBlob(SHADERSTAGE_PIXEL)){
		// no applicable blobs!
		return nullptr;
	}

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
	state.pRootSignature = mRootSignature.Get();
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
	state.RTVFormats[0] = Graphics::GetDisplayFormat();
	state.DSVFormat = Graphics::GetDepthFormat();
	state.SampleDesc = sampDesc;

	ComPtr<ID3D12PipelineState> pso;
	ThrowIfFailed(device->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&pso)));

	delete[] inputElements;
	return pso;
}
void Shader::CreateComputePSO() {
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.CS = CD3DX12_SHADER_BYTECODE(GetBlob(SHADERSTAGE_COMPUTE));
	Graphics::GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mComputePSO));
}

void Shader::Upload() {
	if (mCreated) return;
	
	ID3DBlob* rsblob = GetBlob(SHADERSTAGE_ROOTSIG);
	if (rsblob)
		ThrowIfFailed(Graphics::GetDevice()->CreateRootSignature(1, rsblob->GetBufferPointer(), rsblob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
	
	mCreated = true;
}