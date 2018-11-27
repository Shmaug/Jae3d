#include "ShaderAsset.hpp"
#include "AssetImporter.hpp"
#include "AssetFile.hpp"
#include "IOUtil.hpp"

#include "MemoryStream.hpp"

#include <vector>
#include <d3dcompiler.h>

using namespace std;
using namespace Microsoft::WRL;

ShaderAsset::ShaderAsset(string name) : Asset(name) {}
ShaderAsset::ShaderAsset(string name, MemoryStream &ms) : Asset(name) {
	uint8_t mask = ms.Read<uint8_t>();
	for (int i = 0; i < 6; i++) {
		if (mask & (1 << i)) {
			uint64_t size = ms.Read<uint64_t>();
			if (FAILED(D3DCreateBlob(size, &m_Blobs[i]))) throw exception();
			ms.Read(reinterpret_cast<char*>(m_Blobs[i]->GetBufferPointer()), size);
		}
	}
}
ShaderAsset::~ShaderAsset() {
	for (int i = 0; i < 6; i++)
		if (m_Blobs[i]) {
			m_Blobs[i]->Release();
			m_Blobs[i] = nullptr;
		}
}
uint64_t ShaderAsset::TypeId() { return (uint64_t)AssetFile::TYPEID_SHADER; }

HRESULT ShaderAsset::ReadShaderStage(wstring path, ShaderStage stage) {
	return D3DReadFileToBlob(path.c_str(), &m_Blobs[stage]);
}
HRESULT ShaderAsset::CompileShaderStage(wstring file, string entryPoint, ShaderStage stage) {
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	vector<D3D_SHADER_MACRO> defines;

	LPCSTR profile;

	switch (stage) {
	default:
	case ShaderStage::Vertex:
		profile = "vs_5_1";
		defines.push_back({ "SHADER_STAGE_VERTEX", "" });
		break;
	case ShaderStage::Hull:
		profile = "hs_5_1";
		defines.push_back({ "SHADER_STAGE_HULL", "" });
		break;
	case ShaderStage::Domain:
		profile = "ds_5_1";
		defines.push_back({ "SHADER_STAGE_DOMAIN", "" });
		break;
	case ShaderStage::Geometry:
		profile = "gs_5_1";
		defines.push_back({ "SHADER_STAGE_GEOMETRY", "" });
		break;
	case ShaderStage::Pixel:
		profile = "ps_5_1";
		defines.push_back({ "SHADER_STAGE_PIXEL", "" });
		break;
	case ShaderStage::Compute:
		profile = "cs_5_1";
		defines.push_back({ "SHADER_STAGE_COMPUTE", "" });
		break;
	}

	defines.push_back({ NULL, NULL });

	if (AssetImporter::verbose) {
		int n = 0;
		for (int i = 0; i < file.length(); i++)
			if (file[i] == '\\')
				n = i + 1;
		printf("%S: Compiling %s with %s\n", file.substr(n, file.length() - n).c_str(), entryPoint.c_str(), profile);
	}
	ID3DBlob *errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(file.c_str(), defines.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), profile, flags, 0, &m_Blobs[stage], &errorBlob);
	
	if (errorBlob) {
		const char *msg = reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
		perror(msg);
		errorBlob->Release();
	}
	return hr;
}

void ShaderAsset::WriteData(MemoryStream &ms) {
	Asset::WriteData(ms);
	uint8_t mask = 0;
	for (int i = 0; i < 6; i++)
		if (m_Blobs[i])
			mask |= 1 << i;
	ms.Write(mask);
	for (int i = 0; i < 6; i++) {
		if (m_Blobs[i]) {
			ms.Write((uint64_t)m_Blobs[i]->GetBufferSize());
			ms.Write(reinterpret_cast<const char*>(m_Blobs[i]->GetBufferPointer()), m_Blobs[i]->GetBufferSize());
		}
	}
}