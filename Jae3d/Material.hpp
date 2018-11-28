#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <variant>
#include <memory>
#include <map>
#include <string>
#include <d3d12.h>
#include <DirectXMath.h>

class Shader;
class Texture;
class CommandList;

#define PARAM_TYPES std::shared_ptr<Texture>, float, DirectX::XMFLOAT4

class Material {
public:
	//union Parameter {
	//	std::shared_ptr<Texture> texValue;
	//	float floatValue;
	//	DirectX::XMFLOAT4 vecValue;
	//
	//	Parameter() { texValue = nullptr; }
	//	~Parameter() {}
	//};

	std::string m_Name;

	Material(std::string name, std::shared_ptr<Shader> shader);
	~Material();

	void SetTexture(std::string param, std::shared_ptr<Texture> tex);
	void SetFloat(std::string param, float v);
	void SetVector(std::string param, DirectX::XMFLOAT4 vec);

private:
	friend class CommandList;
	void SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	std::shared_ptr<Shader> m_Shader;
	std::map<std::string, std::variant<PARAM_TYPES>> m_Params;
};

#undef PARAM_TYPES

