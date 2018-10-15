#include "Shader.h"

#include <winbase.h>
#include <locale>
#include <codecvt>
#include <iostream>
#include <fstream>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "Graphics.h"
#include "Util.h"
#include "Mesh.h"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

#pragma region ShaderLibrary
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

void ShaderLibrary::LoadShaders() {
	auto device = Graphics::GetDevice();

	string name = "default";

	Shader *s = new Shader();
	s->name = name;

	ReadIfExists(s2ws("Assets/Shaders/" + name + "_vs.cso").c_str(), &s->vertexBlob);
	ReadIfExists(s2ws("Assets/Shaders/" + name + "_ps.cso").c_str(), &s->pixelBlob);

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_PARAMETER1 rootParams[RootParameters::NumRootParameters];
	rootParams[RootParameters::CameraData].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	//rootParams[RootParameters::ObjectData].InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.Init_1_1(_countof(rootParams), rootParams, 0, nullptr, rootSignatureFlags);

	s->m_RootSignature.SetRootSignatureDesc(rootSigDesc.Desc_1_1, featureData.HighestVersion);

	struct PipelineStateStream {
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		//CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
	} pipelineStateStream;

	D3D12_RT_FORMAT_ARRAY rtvFormats = { };
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	pipelineStateStream.pRootSignature = s->m_RootSignature.GetRootSignature().Get();
	pipelineStateStream.InputLayout = { Vertex::InputElements, Vertex::InputElementCount };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(s->vertexBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(s->pixelBlob.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;
	//pipelineStateStream.SampleDesc = Graphics::GetMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT, D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE);

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { sizeof(PipelineStateStream), &pipelineStateStream };
	ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&s->m_PipelineState)));

	shaders[name] = s;
}
Shader* ShaderLibrary::GetShader(string name) {
	return shaders[name];
}
#pragma endregion

#pragma region RootSignature
RootSignature::RootSignature()
	: m_RootSignatureDesc{}
	, m_NumDescriptorsPerTable{ 0 }
	, m_SamplerTableBitMask(0)
	, m_DescriptorTableBitMask(0)
{}

RootSignature::RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion)
	: m_RootSignatureDesc{}
	, m_NumDescriptorsPerTable{ 0 }
	, m_SamplerTableBitMask(0)
	, m_DescriptorTableBitMask(0)
{
	SetRootSignatureDesc(rootSignatureDesc, rootSignatureVersion);
}

RootSignature::~RootSignature() {
	Destroy();
}

void RootSignature::Destroy() {
	for (UINT i = 0; i < m_RootSignatureDesc.NumParameters; ++i) {
		const D3D12_ROOT_PARAMETER1& rootParameter = m_RootSignatureDesc.pParameters[i];
		if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			delete[] rootParameter.DescriptorTable.pDescriptorRanges;
		}
	}

	delete[] m_RootSignatureDesc.pParameters;
	m_RootSignatureDesc.pParameters = nullptr;
	m_RootSignatureDesc.NumParameters = 0;

	delete[] m_RootSignatureDesc.pStaticSamplers;
	m_RootSignatureDesc.pStaticSamplers = nullptr;
	m_RootSignatureDesc.NumStaticSamplers = 0;

	m_DescriptorTableBitMask = 0;
	m_SamplerTableBitMask = 0;

	memset(m_NumDescriptorsPerTable, 0, sizeof(m_NumDescriptorsPerTable));
}

void RootSignature::SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion) {
	// Make sure any previously allocated root signature description is cleaned 
	// up first.
	Destroy();

	auto device = Graphics::GetDevice();

	UINT numParameters = rootSignatureDesc.NumParameters;
	D3D12_ROOT_PARAMETER1* pParameters = numParameters > 0 ? new D3D12_ROOT_PARAMETER1[numParameters] : nullptr;

	for (UINT i = 0; i < numParameters; ++i) {
		const D3D12_ROOT_PARAMETER1& rootParameter = rootSignatureDesc.pParameters[i];
		pParameters[i] = rootParameter;

		if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			UINT numDescriptorRanges = rootParameter.DescriptorTable.NumDescriptorRanges;
			D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges = numDescriptorRanges > 0 ? new D3D12_DESCRIPTOR_RANGE1[numDescriptorRanges] : nullptr;

			memcpy(pDescriptorRanges, rootParameter.DescriptorTable.pDescriptorRanges,
				sizeof(D3D12_DESCRIPTOR_RANGE1) * numDescriptorRanges);

			pParameters[i].DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
			pParameters[i].DescriptorTable.pDescriptorRanges = pDescriptorRanges;

			// Set the bit mask depending on the type of descriptor table.
			if (numDescriptorRanges > 0) {
				switch (pDescriptorRanges[0].RangeType) {
				case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
					m_DescriptorTableBitMask |= (1 << i);
					break;
				case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
					m_SamplerTableBitMask |= (1 << i);
					break;
				}
			}

			// Count the number of descriptors in the descriptor table.
			for (UINT j = 0; j < numDescriptorRanges; ++j) {
				m_NumDescriptorsPerTable[i] += pDescriptorRanges[j].NumDescriptors;
			}
		}
	}

	m_RootSignatureDesc.NumParameters = numParameters;
	m_RootSignatureDesc.pParameters = pParameters;

	UINT numStaticSamplers = rootSignatureDesc.NumStaticSamplers;
	D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = numStaticSamplers > 0 ? new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers] : nullptr;

	if (pStaticSamplers) {
		memcpy(pStaticSamplers, rootSignatureDesc.pStaticSamplers,
			sizeof(D3D12_STATIC_SAMPLER_DESC) * numStaticSamplers);
	}

	m_RootSignatureDesc.NumStaticSamplers = numStaticSamplers;
	m_RootSignatureDesc.pStaticSamplers = pStaticSamplers;

	D3D12_ROOT_SIGNATURE_FLAGS flags = rootSignatureDesc.Flags;
	m_RootSignatureDesc.Flags = flags;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionRootSignatureDesc;
	versionRootSignatureDesc.Init_1_1(numParameters, pParameters, numStaticSamplers, pStaticSamplers, flags);

	// Serialize the root signature.
	Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&versionRootSignatureDesc,
		rootSignatureVersion, &rootSignatureBlob, &errorBlob));

	// Create the root signature.
	ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
}

uint32_t RootSignature::GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const {
	uint32_t descriptorTableBitMask = 0;
	switch (descriptorHeapType) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		descriptorTableBitMask = m_DescriptorTableBitMask;
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		descriptorTableBitMask = m_SamplerTableBitMask;
		break;
	}

	return descriptorTableBitMask;
}

uint32_t RootSignature::GetNumDescriptors(uint32_t rootIndex) const {
	assert(rootIndex < 32);
	return m_NumDescriptorsPerTable[rootIndex];
}
#pragma endregion