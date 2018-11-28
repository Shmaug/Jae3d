#pragma once

#include "Asset.hpp"

#include <map>
#include <string>
#include <d3d12.h>

#include <d3d12.h>

class RootSignature;

class ShaderAsset : public Asset {
public:
	enum SHADERSTAGE {
		SHADERSTAGE_ROOTSIG,
		SHADERSTAGE_VERTEX,
		SHADERSTAGE_HULL,
		SHADERSTAGE_DOMAIN,
		SHADERSTAGE_GEOMETRY,
		SHADERSTAGE_PIXEL,
		SHADERSTAGE_COMPUTE
	};

	enum PARAM_TYPE {
		PARAM_TYPE_CBV,
		PARAM_TYPE_SRV,
		PARAM_TYPE_UAV,
		PARAM_TYPE_SAMPLER,
	};
	struct Parameter {
	private:
		PARAM_TYPE type;
		unsigned int index;

	public:
		Parameter(PARAM_TYPE type, unsigned int index)
			: type(type), index(index) {}
		PARAM_TYPE Type() const { return type; }
		unsigned int Index() const { return index; }
	};

	ShaderAsset(std::string name);
	ShaderAsset(std::string name, MemoryStream &ms);
	~ShaderAsset();

	HRESULT ReadShaderStage(std::wstring path, SHADERSTAGE stage);
	HRESULT CompileShaderStage(std::wstring path, std::string entryPoint, SHADERSTAGE stage);

	void WriteData(MemoryStream &ms);
	uint64_t TypeId();

	ID3DBlob* GetBlob(SHADERSTAGE stage) { return m_Blobs[stage]; }

	Parameter* GetParameter(std::string name) { return &m_Params.at(name); }
	void AddParameter(std::string name, Parameter &param) { m_Params.emplace(name, param); }

private:
	std::map<std::string, Parameter> m_Params;
	ID3DBlob* m_Blobs[7] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
};

