#include "ShaderAsset.hpp"
#include "AssetImporter.hpp"
#include "Util.hpp"

#include <vector>
#include <d3dcompiler.h>

using namespace std;

ShaderAsset::ShaderAsset(string name) : Asset(name) {}
ShaderAsset::~ShaderAsset() {
	for (int i = 0; i < 6; i++)
		if (m_Blobs[i])
			m_Blobs[i]->Release();
}

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

	if (AssetImporter::verbose)
		printf("   Compiling %s with %s\n", entryPoint.c_str(), profile);

	ID3DBlob *errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(file.c_str(), defines.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), profile, flags, 0, &m_Blobs[stage], &errorBlob);
	
	if (errorBlob) {
		const char *msg = reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
		perror(msg);
		errorBlob->Release();
	}
	return hr;
}

void ShaderAsset::WriteHeader(ofstream &stream) {
	Asset::WriteHeader(stream);
	//WriteStream(stream, "SHADER");
	//for (int i = 0; i < 6; i++)
	//	if (m_Blobs[i])
	//		WriteStream(stream, stageEnum[i]);
}
void ShaderAsset::WriteData(ofstream &stream) {
	Asset::WriteData(stream);
	//WriteStream(stream, "SHADER");
	//
	//for (int i = 0; i < 6; i++) {
	//	if (m_Blobs[i]) {
	//		WriteStream(stream, stageEnum[i]);
	//		WriteStream(stream, (uint64_t)m_Blobs[i]->GetBufferSize());
	//		stream.write(reinterpret_cast<const char*>(m_Blobs[i]->GetBufferPointer()), m_Blobs[i]->GetBufferSize());
	//	}
	//}
}