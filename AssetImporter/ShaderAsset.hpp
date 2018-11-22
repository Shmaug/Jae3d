#pragma once

#include "Asset.hpp"

#include <string>
#include <d3d12.h>

#ifdef DOMAIN
#undef DOMAIN
#endif

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
	~ShaderAsset();

	HRESULT ReadShaderStage(std::wstring path, ShaderStage stage);
	HRESULT CompileShaderStage(std::wstring path, std::string entryPoint, ShaderStage stage);

	void WriteHeader(std::ofstream &stream);
	void WriteData(std::ofstream &stream);

private:
	ID3DBlob* m_Blobs[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	const char* stageEnum[6] = {
		"VERTEX",
		"HULL",
		"DOMAIN",
		"GEOMETRY",
		"PIXEL",
		"COMPUTE"
	};
};

