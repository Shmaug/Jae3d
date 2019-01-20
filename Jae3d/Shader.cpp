#include "Shader.hpp"

#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <iostream>
#include <exception>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "Graphics.hpp"
#include "Mesh.hpp"
#include "Window.hpp"

#include "MemoryStream.hpp"
#include "AssetFile.hpp"

#include "IOUtil.hpp"

#include <vector>

using namespace DirectX;
using namespace Microsoft::WRL;

class CustomInclude : public ID3DInclude {
public:
	jstring mDir;
	jvector<jstring> mPaths;
	CustomInclude(jstring dir, jvector<jstring> paths) : mDir(dir), mPaths(paths) {}

	HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override {
		jstring filePath = pFileName;
		if (IncludeType == D3D_INCLUDE_SYSTEM) {
			for (unsigned int i = 0; i < mPaths.size(); i++) {
				jstring p = mPaths[i] + "\\" + filePath;
				if (PathFileExistsA(p.c_str())) {
					filePath = p;
					break;
				}
			}
		}else if (IncludeType == D3D_INCLUDE_LOCAL){
			filePath = mDir + "\\" + filePath;
		} else
			return E_FAIL;

		if (PathFileExistsA(filePath.c_str()) == false)
			return E_FAIL;

		auto file = std::ifstream(filePath.c_str(), std::ios::binary);
		file.seekg(0, std::ios::end);
		auto p = file.tellg();
		file.seekg(0, std::ios::beg);

		*pBytes = UINT(p - file.tellg());
		char* data = new char[*pBytes];

		file.read(data, *pBytes);

		*ppData = data;
		return S_OK;
	}

	HRESULT Close(LPCVOID pData) override {
		delete[] pData;
		return S_OK;
	}
};

jstring Shader::KeywordListToString(jvector<jstring> keywords) {
	for (unsigned int i = 0; i < keywords.size(); i++) {
		if (mKeywords.count(i) == 0 || keywords[i].empty()) {
			keywords.remove(i);
			i--;
		}
	}
	if (keywords.size() == 0) return "";

	std::sort(keywords.data(), keywords.data() + keywords.size(), [](const jstring& lhs, const jstring& rhs) { return strcmp(lhs.c_str(), rhs.c_str()) < 0; });

	jstring str;
	for (unsigned int i = 0; i < keywords.size(); i++)
		str += keywords[i] + " ";

	return str;
}

Shader::Shader(jwstring name) : Asset(name) {}
Shader::Shader(jwstring name, MemoryStream &ms) : Asset(name, ms) {
	uint32_t count = ms.Read<uint32_t>();
	for (unsigned int j = 0; j < count; j++) {
		uint8_t mask = ms.Read<uint8_t>();
		jstring keywords = ms.ReadStringA();

		ComPtr<ID3DBlob>* blobs = new ComPtr<ID3DBlob>[7];
		ZeroMemory(blobs, 7 * sizeof(ComPtr<ID3DBlob>));

		for (int i = 0; i < 7; i++) {
			if (mask & (1 << i)) {
				uint64_t size = ms.Read<uint64_t>();
				if (FAILED(D3DCreateBlob(size, &blobs[i])))
					throw std::exception();
				ms.Read(reinterpret_cast<char*>(blobs[i]->GetBufferPointer()), size);
			}
		}

		mBlobs.emplace(keywords, blobs);
	}

	uint32_t kwcount = ms.Read<uint32_t>();
	for (unsigned int i = 0; i < kwcount; i++)
		mKeywords.emplace(ms.ReadStringA());

	uint32_t pcount = ms.Read<uint32_t>();
	for (unsigned int i = 0; i < pcount; i++) {
		jstring name = ms.ReadStringA();
		SHADER_PARAM_TYPE type = (SHADER_PARAM_TYPE)ms.Read<uint32_t>();
		uint32_t index = ms.Read<uint32_t>();
		uint32_t offset = ms.Read<uint32_t>();
		ShaderParameter::ShaderValue value = ms.Read<ShaderParameter::ShaderValue>();

		AddParameter(name, ShaderParameter(type, index, offset, value));
	}

	uint32_t pbcount = ms.Read<uint32_t>();
	for (unsigned int i = 0; i < pbcount; i++) {
		uint32_t index = ms.Read<uint32_t>();
		uint32_t size = ms.Read<uint32_t>();
		mCBufferParameters.push_back(ShaderParameterBuffer(index, size));
	}
}
Shader::~Shader() {
	for (const auto &it : mBlobs)
		delete[] it.second;
	mBlobs.clear();
	mParams.clear();
	mStates.clear();
}
uint64_t Shader::TypeId() { return (uint64_t)ASSET_TYPE_SHADER; }

bool Shader::SetActive(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	if (!mCreated) Upload();
	if (mRootSignature) {
		commandList->SetGraphicsRootSignature(mRootSignature.Get());
		return true;
	}
	return false;
}

void Shader::SetPSO(ComPtr<ID3D12GraphicsCommandList2> commandList, const ShaderState &state) {
	if (mStates.count(state) == 0)
		mStates.emplace(state, CreatePSO(state));
	commandList->SetPipelineState(mStates.at(state).Get());
}

void Shader::SetCompute(ComPtr<ID3D12GraphicsCommandList2> commandList, const ShaderState &state) {
	if (!mCreated) Upload();

	if (mRootSignature) {
		commandList->SetComputeRootSignature(mRootSignature.Get());

		if (!mComputePSO) CreateComputePSO(state);
		commandList->SetPipelineState(mComputePSO.Get());
	}
}

ComPtr<ID3D12PipelineState> Shader::CreatePSO(const ShaderState &state) {
	ComPtr<ID3DBlob>* blobs = mBlobs.at(KeywordListToString(state.keywords));

	if (!blobs[SHADER_STAGE_VERTEX] && !blobs[SHADER_STAGE_HULL] &&
		!blobs[SHADER_STAGE_DOMAIN] && !blobs[SHADER_STAGE_GEOMETRY] && !blobs[SHADER_STAGE_PIXEL]){
		// no applicable shaders!
		return nullptr;
	}

	UINT inputElementCount = 0;
	inputElementCount++; // position
	if (state.input & MESH_SEMANTIC_NORMAL) inputElementCount++;
	if (state.input & MESH_SEMANTIC_TANGENT) inputElementCount++;
	if (state.input & MESH_SEMANTIC_BINORMAL) inputElementCount++;
	if (state.input & MESH_SEMANTIC_COLOR0) inputElementCount++;
	if (state.input & MESH_SEMANTIC_COLOR1) inputElementCount++;
	if (state.input & MESH_SEMANTIC_BLENDINDICES) inputElementCount++;
	if (state.input & MESH_SEMANTIC_BLENDWEIGHT) inputElementCount++;
	if (state.input & MESH_SEMANTIC_TEXCOORD0) inputElementCount++;
	if (state.input & MESH_SEMANTIC_TEXCOORD1) inputElementCount++;
	if (state.input & MESH_SEMANTIC_TEXCOORD2) inputElementCount++;
	if (state.input & MESH_SEMANTIC_TEXCOORD3) inputElementCount++;

	D3D12_INPUT_ELEMENT_DESC* inputElements = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
	int i = 0;
	inputElements[i++] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_NORMAL)
		inputElements[i++] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_TANGENT)
		inputElements[i++] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_BINORMAL)
		inputElements[i++] = { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_COLOR0)
		inputElements[i++] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_COLOR1)
		inputElements[i++] = { "COLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_BLENDINDICES)
		inputElements[i++] = { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_BLENDWEIGHT)
		inputElements[i++] = { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_TEXCOORD0)
		inputElements[i++] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_TEXCOORD1)
		inputElements[i++] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_TEXCOORD2)
		inputElements[i++] = { "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	if (state.input & MESH_SEMANTIC_TEXCOORD3)
		inputElements[i++] = { "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	DXGI_SAMPLE_DESC sampDesc = {};
	sampDesc.Count = state.msaaSamples;
	sampDesc.Quality = 0;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoState = {};
	psoState.InputLayout = { inputElements, inputElementCount };
	psoState.pRootSignature = mRootSignature.Get();
	if (blobs[SHADER_STAGE_VERTEX])		psoState.VS = CD3DX12_SHADER_BYTECODE(blobs[SHADER_STAGE_VERTEX].Get());
	if (blobs[SHADER_STAGE_HULL])		psoState.HS = CD3DX12_SHADER_BYTECODE(blobs[SHADER_STAGE_HULL].Get());
	if (blobs[SHADER_STAGE_DOMAIN])		psoState.DS = CD3DX12_SHADER_BYTECODE(blobs[SHADER_STAGE_DOMAIN].Get());
	if (blobs[SHADER_STAGE_GEOMETRY])	psoState.GS = CD3DX12_SHADER_BYTECODE(blobs[SHADER_STAGE_GEOMETRY].Get());
	if (blobs[SHADER_STAGE_PIXEL])		psoState.PS = CD3DX12_SHADER_BYTECODE(blobs[SHADER_STAGE_PIXEL].Get());
	psoState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	if (state.topology == D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE || state.fillMode == D3D12_FILL_MODE_WIREFRAME)
		psoState.RasterizerState.AntialiasedLineEnable = TRUE;
	psoState.RasterizerState.FillMode = state.fillMode;
	psoState.RasterizerState.CullMode = state.cullMode;
	psoState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoState.BlendState.RenderTarget[0] = state.blendState;
	psoState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	if (!state.ztest && !state.zwrite)
		psoState.DepthStencilState.DepthEnable = false;
	else {
		psoState.DepthStencilState.DepthWriteMask = state.zwrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		psoState.DepthStencilState.DepthFunc = state.ztest ? D3D12_COMPARISON_FUNC_LESS_EQUAL : D3D12_COMPARISON_FUNC_ALWAYS;
	}
	psoState.SampleMask = UINT_MAX;
	psoState.PrimitiveTopologyType = state.topology;
	psoState.NumRenderTargets = 1;
	psoState.RTVFormats[0] = state.renderFormat;
	psoState.DSVFormat = state.depthFormat;
	psoState.SampleDesc = sampDesc;

	ComPtr<ID3D12PipelineState> pso;
	ThrowIfFailed(Graphics::GetDevice()->CreateGraphicsPipelineState(&psoState, IID_PPV_ARGS(&pso)));

	delete[] inputElements;
	return pso;
}
void Shader::CreateComputePSO(const ShaderState &state) {
	jstring key = KeywordListToString(state.keywords);

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.CS = CD3DX12_SHADER_BYTECODE(mBlobs.at(key)[SHADER_STAGE_COMPUTE].Get());
	Graphics::GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mComputePSO));
}

void Shader::Upload() {
	if (mCreated) return;
	
	ComPtr<ID3DBlob> rsblob = mBlobs.at("")[SHADER_STAGE_ROOTSIG];
	if (rsblob) {
		ThrowIfFailed(Graphics::GetDevice()->CreateRootSignature(1, rsblob->GetBufferPointer(), rsblob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
		mRootSignature->SetName((mName + L" Root Signature").c_str());
	}
	
	mCreated = true;
}

HRESULT Shader::CompileShaderStage(jwstring file, jstring entryPoint, SHADER_STAGE stage, jvector<jstring> &includePaths, jvector<jstring> &keywords) {
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	jvector<D3D_SHADER_MACRO> defines;

	LPCSTR profile;

	switch (stage) {
	default:
	case SHADER_STAGE_ROOTSIG:
		profile = "rootsig_1_1";
		defines.push_back({ "" });
		break;
	case SHADER_STAGE_VERTEX:
		profile = "vs_5_1";
		defines.push_back({ "SHADER_STAGE_VERTEX", "" });
		break;
	case SHADER_STAGE_HULL:
		profile = "hs_5_1";
		defines.push_back({ "SHADER_STAGE_HULL", "" });
		break;
	case SHADER_STAGE_DOMAIN:
		profile = "ds_5_1";
		defines.push_back({ "SHADER_STAGE_DOMAIN", "" });
		break;
	case SHADER_STAGE_GEOMETRY:
		profile = "gs_5_1";
		defines.push_back({ "SHADER_STAGE_GEOMETRY", "" });
		break;
	case SHADER_STAGE_PIXEL:
		profile = "ps_5_1";
		defines.push_back({ "SHADER_STAGE_PIXEL", "" });
		break;
	case SHADER_STAGE_COMPUTE:
		profile = "cs_5_1";
		defines.push_back({ "SHADER_STAGE_COMPUTE", "" });
		break;
	}

	for (unsigned int i = 0; i < keywords.size(); i++) {
		mKeywords.emplace(keywords[i]);
		defines.push_back({ keywords[i].c_str(), "" });
	}

	defines.push_back({ NULL, NULL });

	jstring key = KeywordListToString(keywords);
	ComPtr<ID3DBlob>* blobs;
	if (mBlobs.count(key))
		blobs = mBlobs.at(key);
	else {
		blobs = new ComPtr<ID3DBlob>[7];
		ZeroMemory(blobs, 7 * sizeof(ComPtr<ID3DBlob>));
		mBlobs.emplace(key, blobs);
	}

	CustomInclude inc(GetDirectoryA(GetFullPathA(utf16toUtf8(file))), includePaths);
	ID3DBlob *errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(file.c_str(), defines.data(), &inc, entryPoint.c_str(), profile, flags, 0, &blobs[stage], &errorBlob);

	if (errorBlob) {
		char msg[128];
		sprintf_s(msg, "Error compiling %S with stage %s\n", GetNameExtW(file).c_str(), profile);
		std::cerr << msg;
		std::cerr << reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
		errorBlob->Release();
		blobs[stage].Reset();
	}

	return hr;
}
HRESULT Shader::CompileShaderStage(const char* text, const char* entryPoint, SHADER_STAGE stage, jvector<jstring> &keywords) {
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	jvector<D3D_SHADER_MACRO> defines;

	LPCSTR profile;

	switch (stage) {
	default:
	case SHADER_STAGE_ROOTSIG:
		profile = "rootsig_1_1";
		defines.push_back({ "" });
		break;
	case SHADER_STAGE_VERTEX:
		profile = "vs_5_1";
		defines.push_back({ "SHADER_STAGE_VERTEX", "" });
		break;
	case SHADER_STAGE_HULL:
		profile = "hs_5_1";
		defines.push_back({ "SHADER_STAGE_HULL", "" });
		break;
	case SHADER_STAGE_DOMAIN:
		profile = "ds_5_1";
		defines.push_back({ "SHADER_STAGE_DOMAIN", "" });
		break;
	case SHADER_STAGE_GEOMETRY:
		profile = "gs_5_1";
		defines.push_back({ "SHADER_STAGE_GEOMETRY", "" });
		break;
	case SHADER_STAGE_PIXEL:
		profile = "ps_5_1";
		defines.push_back({ "SHADER_STAGE_PIXEL", "" });
		break;
	case SHADER_STAGE_COMPUTE:
		profile = "cs_5_1";
		defines.push_back({ "SHADER_STAGE_COMPUTE", "" });
		break;
	}

	for (unsigned int i = 0; i < keywords.size(); i++) {
		mKeywords.emplace(keywords[i]);
		defines.push_back({ keywords[i].c_str(), "" });
	}

	defines.push_back({ NULL, NULL });

	jstring key = KeywordListToString(keywords);
	ComPtr<ID3DBlob>* blobs;
	if (mBlobs.count(key))
		blobs = mBlobs.at(key);
	else {
		blobs = new ComPtr<ID3DBlob>[7];
		ZeroMemory(blobs, 7 * sizeof(ComPtr<ID3DBlob>));
	}

	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompile2(text, strlen(text), NULL, defines.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, 0, NULL, 0, &blobs[stage], &errorBlob);

	if (errorBlob) {
		std::cerr << "Error compiling " << profile << "\n";
		std::cerr << reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
		OutputDebugf(L"Error compiling %S\n", profile);
		OutputDebugf(L"%S\n", reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
		errorBlob->Release();
	}

	if (FAILED(hr)) {
		if (blobs[stage]) blobs[stage].Reset();
		blobs[stage] = nullptr;
	}

	return hr;
}

void Shader::WriteData(MemoryStream &ms) {
	ms.Write((uint32_t)mBlobs.size());
	for (const auto &it : mBlobs) {
		uint8_t mask = 0;
		for (unsigned int i = 0; i < 7; i++)
			if (it.second[i])
				mask |= 1 << i;
		ms.Write(mask);
		ms.WriteStringA(it.first);
		printf("  variant: '%s'\n", it.first.c_str());
		for (unsigned int i = 0; i < 7; i++) {
			if (it.second[i]) {
				ms.Write((uint64_t)it.second[i]->GetBufferSize());
				ms.Write(reinterpret_cast<const char*>(it.second[i]->GetBufferPointer()), it.second[i]->GetBufferSize());
			}
		}
	}

	printf("  all: ");
	ms.Write((uint32_t)mKeywords.size());
	for (const auto &it : mKeywords) {
		ms.WriteStringA(it);
		printf("'%s' ", it.c_str());
	}
	printf("\n");

	ms.Write((uint32_t)mParams.size());
	for (const auto& it : mParams){
		ms.WriteStringA(it.first);
		ms.Write((uint32_t)it.second.Type());
		ms.Write(it.second.RootIndex());
		ms.Write(it.second.CBufferOffset());
		ms.Write(it.second.GetDefaultValue());
	}

	ms.Write((uint32_t)mCBufferParameters.size());
	for (unsigned int i = 0; i < mCBufferParameters.size(); i++) {
		ms.Write((uint32_t)mCBufferParameters[i].rootIndex);
		ms.Write((uint32_t)mCBufferParameters[i].size);
	}
}