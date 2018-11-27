#pragma once

#include "Asset.hpp"

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <string>
#include <d3d12.h>

#include <d3d12.h>

class RootSignature;

class ShaderAsset : public Asset {
public:
	enum ShaderStage {
		Vertex		= 1,
		Hull		= 2,
		Domain		= 3,
		Geometry	= 4,
		Pixel		= 5,
		Compute		= 6
	};

	ShaderAsset(std::string name);
	ShaderAsset(std::string name, MemoryStream &ms);
	~ShaderAsset();

	HRESULT ReadShaderStage(std::wstring path, ShaderStage stage);
	HRESULT CompileShaderStage(std::wstring path, std::string entryPoint, ShaderStage stage);

	void WriteData(MemoryStream &ms);
	uint64_t TypeId();

private:
	ID3DBlob* m_Blobs[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
};

