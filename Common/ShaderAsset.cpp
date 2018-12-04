#include "ShaderAsset.hpp"
#include "AssetFile.hpp"
#include "IOUtil.hpp"

#include "MemoryStream.hpp"
#include "../Jae3d/Util.hpp"

#include <d3dcompiler.h>

using namespace std;
using namespace DirectX;

ShaderAsset::ShaderAsset(jstring name) : Asset(name) {}
ShaderAsset::ShaderAsset(jstring name, MemoryStream &ms) : Asset(name) {
	uint8_t mask = ms.Read<uint8_t>();
	for (int i = 0; i < 7; i++) {
		if (mask & (1 << i)) {
			uint64_t size = ms.Read<uint64_t>();
			if (FAILED(D3DCreateBlob(size, &mBlobs[i]))) throw exception();
			ms.Read(reinterpret_cast<char*>(mBlobs[i]->GetBufferPointer()), size);
		}
	}

	uint32_t pcount = ms.Read<uint32_t>();
	for (unsigned int i = 0; i < pcount; i++) {
		jstring name = ms.ReadString();
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
ShaderAsset::~ShaderAsset() {
	for (int i = 0; i < 7; i++)
		if (mBlobs[i]) {
			mBlobs[i]->Release();
			mBlobs[i] = nullptr;
		}
	mParams.clear();
}
uint64_t ShaderAsset::TypeId() { return (uint64_t)AssetFile::TYPEID_SHADER; }

HRESULT ShaderAsset::ReadShaderStage(jwstring path, SHADERSTAGE stage) {
	return D3DReadFileToBlob(path.c_str(), &mBlobs[stage]);
}
HRESULT ShaderAsset::CompileShaderStage(jwstring file, jstring entryPoint, SHADERSTAGE stage) {
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	vector<D3D_SHADER_MACRO> defines;

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

	//if (AssetImporter::verbose) {
	//	int n = 0;
	//	for (int i = 0; i < file.length(); i++)
	//		if (file[i] == '\\')
	//			n = i + 1;
	//	printf("%S: Compiling %s with %s\n", file.substr(n, file.length() - n).c_str(), entryPoint.c_str(), profile);
	//}

	ID3DBlob *errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(file.c_str(), defines.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), profile, flags, 0, &mBlobs[stage], &errorBlob);
	
	if (errorBlob) {
		char msg[128];
		sprintf_s(msg, "Error compiling %s with stage %s\n", GetNameExt(utf16toUtf8(file)).c_str(), profile);
		cerr << msg;
		cerr << reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
		errorBlob->Release();
	}
	return hr;
}

void ShaderAsset::WriteData(MemoryStream &ms) {
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