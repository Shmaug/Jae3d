#pragma once

#include "Asset.hpp"

#include <string>
#include <d3d12.h>

#include <d3d12.h>

class RootSignature;

class ShaderAsset : public Asset {
public:
	enum SHADERSTAGE {
		SHADERSTAGE_VERTEX		= 1,
		SHADERSTAGE_HULL		= 2,
		SHADERSTAGE_DOMAIN		= 3,
		SHADERSTAGE_GEOMETRY	= 4,
		SHADERSTAGE_PIXEL		= 5,
		SHADERSTAGE_COMPUTE		= 6
	};

	ShaderAsset(std::string name);
	ShaderAsset(std::string name, MemoryStream &ms);
	~ShaderAsset();

	HRESULT ReadShaderStage(std::wstring path, SHADERSTAGE stage);
	HRESULT CompileShaderStage(std::wstring path, std::string entryPoint, SHADERSTAGE stage);

	void WriteData(MemoryStream &ms);
	uint64_t TypeId();

	ID3DBlob* GetBlob(SHADERSTAGE stage) { return m_Blobs[stage]; }

private:
	ID3DBlob* m_Blobs[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
};

