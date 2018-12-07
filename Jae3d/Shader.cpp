#include "Shader.hpp"

#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <iostream>
#include <exception>

#include "Graphics.hpp"
#include "Mesh.hpp"
#include "Window.hpp"

#include "MemoryStream.hpp"
#include "AssetFile.hpp"

using namespace DirectX;
using namespace Microsoft::WRL;

Shader::Shader(jwstring name) : Asset(name) {}
Shader::Shader(jwstring name, MemoryStream &ms) : Asset(name) {
	uint8_t mask = ms.Read<uint8_t>();
	for (int i = 0; i < 7; i++) {
		if (mask & (1 << i)) {
			uint64_t size = ms.Read<uint64_t>();
			if (FAILED(D3DCreateBlob(size, &mBlobs[i]))) throw std::exception();
			ms.Read(reinterpret_cast<char*>(mBlobs[i]->GetBufferPointer()), size);
		}
	}

	uint32_t pcount = ms.Read<uint32_t>();
	for (unsigned int i = 0; i < pcount; i++) {
		jwstring name = ms.ReadString();
		PARAM_TYPE type = (PARAM_TYPE)ms.Read<uint32_t>();
		uint32_t index = ms.Read<uint32_t>();
		uint32_t offset = ms.Read<uint32_t>();
		ShaderParameter::ShaderValue value = ms.Read<ShaderParameter::ShaderValue>();

		AddParameter(name, ShaderParameter(type, index, offset, value));
	}

	uint32_t pbcount = ms.Read<uint32_t>();
	for (unsigned int i = 0; i < pbcount; i++) {
		uint32_t index = ms.Read<uint32_t>();
		uint32_t size = ms.Read<uint32_t>();
		mCBufferParameters.push_back(ParameterBuffer(index, size));
	}
}
Shader::~Shader() {
	for (int i = 0; i < 7; i++)
		if (mBlobs[i]) {
			mBlobs[i]->Release();
			mBlobs[i] = nullptr;
		}
	mParams.clear();
	mStates.clear();
}
uint64_t Shader::TypeId() { return (uint64_t)AssetFile::TYPEID_SHADER; }

// Set the root signature on the GPU
bool Shader::SetActive(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!mCreated) Upload();
	if (mRootSignature) {
		commandList->SetGraphicsRootSignature(mRootSignature.Get());
		return true;
	}
	return false;
}

void Shader::SetPSO(ComPtr<ID3D12GraphicsCommandList2> commandList, Mesh::SEMANTIC input) {
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

ComPtr<ID3D12PipelineState> Shader::CreatePSO(Mesh::SEMANTIC input) {
	auto device = Graphics::GetDevice();

	if (!mBlobs[SHADERSTAGE_VERTEX] && !mBlobs[SHADERSTAGE_HULL] &&
		!mBlobs[SHADERSTAGE_DOMAIN] && !mBlobs[SHADERSTAGE_GEOMETRY] && !mBlobs[SHADERSTAGE_PIXEL]){
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
	if (mBlobs[SHADERSTAGE_VERTEX])		state.VS = CD3DX12_SHADER_BYTECODE(mBlobs[SHADERSTAGE_VERTEX]);
	if (mBlobs[SHADERSTAGE_HULL])		state.HS = CD3DX12_SHADER_BYTECODE(mBlobs[SHADERSTAGE_HULL]);
	if (mBlobs[SHADERSTAGE_DOMAIN])		state.DS = CD3DX12_SHADER_BYTECODE(mBlobs[SHADERSTAGE_DOMAIN]);
	if (mBlobs[SHADERSTAGE_GEOMETRY])	state.GS = CD3DX12_SHADER_BYTECODE(mBlobs[SHADERSTAGE_GEOMETRY]);
	if (mBlobs[SHADERSTAGE_PIXEL])		state.PS = CD3DX12_SHADER_BYTECODE(mBlobs[SHADERSTAGE_PIXEL]);
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
	psoDesc.CS = CD3DX12_SHADER_BYTECODE(mBlobs[SHADERSTAGE_COMPUTE]);
	Graphics::GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mComputePSO));
}

void Shader::Upload() {
	if (mCreated) return;
	
	ID3DBlob* rsblob = mBlobs[SHADERSTAGE_ROOTSIG];
	if (rsblob)
		ThrowIfFailed(Graphics::GetDevice()->CreateRootSignature(1, rsblob->GetBufferPointer(), rsblob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
	
	mCreated = true;
}

HRESULT Shader::ReadShaderStage(jwstring path, SHADERSTAGE stage) {
	return D3DReadFileToBlob(path.c_str(), &mBlobs[stage]);
}
HRESULT Shader::CompileShaderStage(jwstring file, jwstring entryPoint, SHADERSTAGE stage) {
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	jvector<D3D_SHADER_MACRO> defines;

	LPCSTR profile;

	switch (stage) {
	default:
	case SHADERSTAGE_ROOTSIG:
		profile = "rootsig_1_1";
		defines.push_back({ "" });
		break;
	case SHADERSTAGE_VERTEX:
		profile = "vs_5_1";
		defines.push_back({ "SHADER_STAGE_VERTEX", "" });
		break;
	case SHADERSTAGE_HULL:
		profile = "hs_5_1";
		defines.push_back({ "SHADER_STAGE_HULL", "" });
		break;
	case SHADERSTAGE_DOMAIN:
		profile = "ds_5_1";
		defines.push_back({ "SHADER_STAGE_DOMAIN", "" });
		break;
	case SHADERSTAGE_GEOMETRY:
		profile = "gs_5_1";
		defines.push_back({ "SHADER_STAGE_GEOMETRY", "" });
		break;
	case SHADERSTAGE_PIXEL:
		profile = "ps_5_1";
		defines.push_back({ "SHADER_STAGE_PIXEL", "" });
		break;
	case SHADERSTAGE_COMPUTE:
		profile = "cs_5_1";
		defines.push_back({ "SHADER_STAGE_COMPUTE", "" });
		break;
	}

	defines.push_back({ NULL, NULL });

	ID3DBlob *errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(file.c_str(), defines.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, utf16toUtf8(entryPoint).c_str(), profile, flags, 0, &mBlobs[stage], &errorBlob);

	if (errorBlob) {
		char msg[128];
		sprintf_s(msg, "Error compiling %s with stage %s\n", GetNameExt(utf16toUtf8(file)).c_str(), profile);
		std::cerr << msg;
		std::cerr << reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
		errorBlob->Release();
	}
	return hr;
}

HRESULT Shader::CompileShaderStage(const char* text, const char* entryPoint, SHADERSTAGE stage) {
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	jvector<D3D_SHADER_MACRO> defines;

	LPCSTR profile;

	switch (stage) {
	default:
	case SHADERSTAGE_ROOTSIG:
		profile = "rootsig_1_1";
		defines.push_back({ "" });
		break;
	case SHADERSTAGE_VERTEX:
		profile = "vs_5_1";
		defines.push_back({ "SHADER_STAGE_VERTEX", "" });
		break;
	case SHADERSTAGE_HULL:
		profile = "hs_5_1";
		defines.push_back({ "SHADER_STAGE_HULL", "" });
		break;
	case SHADERSTAGE_DOMAIN:
		profile = "ds_5_1";
		defines.push_back({ "SHADER_STAGE_DOMAIN", "" });
		break;
	case SHADERSTAGE_GEOMETRY:
		profile = "gs_5_1";
		defines.push_back({ "SHADER_STAGE_GEOMETRY", "" });
		break;
	case SHADERSTAGE_PIXEL:
		profile = "ps_5_1";
		defines.push_back({ "SHADER_STAGE_PIXEL", "" });
		break;
	case SHADERSTAGE_COMPUTE:
		profile = "cs_5_1";
		defines.push_back({ "SHADER_STAGE_COMPUTE", "" });
		break;
	}

	defines.push_back({ NULL, NULL });

	ID3DBlob *errorBlob = nullptr;
	HRESULT hr = D3DCompile2(text, strlen(text), NULL, defines.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, 0, NULL, 0, &mBlobs[stage], &errorBlob);

	if (errorBlob) {
		std::cerr << "Error compiling " << profile << "\n";
		std::cerr << reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
		errorBlob->Release();
	}

	if (FAILED(hr)) {
		if (mBlobs[stage]) mBlobs[stage]->Release();
		mBlobs[stage] = nullptr;
	}
	return hr;
}

void Shader::WriteData(MemoryStream &ms) {
	Asset::WriteData(ms);
	uint8_t mask = 0;
	for (int i = 0; i < 7; i++)
		if (mBlobs[i])
			mask |= 1 << i;
	ms.Write(mask);
	for (int i = 0; i < 7; i++) {
		if (mBlobs[i]) {
			ms.Write((uint64_t)mBlobs[i]->GetBufferSize());
			ms.Write(reinterpret_cast<const char*>(mBlobs[i]->GetBufferPointer()), mBlobs[i]->GetBufferSize());
		}
	}

	size_t pos = ms.Tell();
	int i = 0;
	ms.Write((uint32_t)0);
	if (!mParams.empty()) {
		for (auto& kp : mParams) {
			ms.WriteString(kp.Key());
			ms.Write((uint32_t)kp.Value().Type());
			ms.Write(kp.Value().RootIndex());
			ms.Write(kp.Value().CBufferOffset());
			ms.Write(kp.Value().GetDefaultValue());
			i++;
		}
		size_t posc = ms.Tell();
		ms.Seek(pos);
		ms.Write((uint32_t)i);
		ms.Seek(posc);
	}

	ms.Write((uint32_t)mCBufferParameters.size());
	for (unsigned int i = 0; i < mCBufferParameters.size(); i++) {
		ms.Write((uint32_t)mCBufferParameters[i].rootIndex);
		ms.Write((uint32_t)mCBufferParameters[i].size);
	}
}