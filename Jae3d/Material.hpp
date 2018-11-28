#pragma once

#include <wrl.h>
#define _WRL Microsoft::WRL

#include <memory>
#include <map>
#include <string>
#include <d3d12.h>
#include <DirectXMath.h>

class Shader;
class Texture;
class CommandList;

class Material {
public:
	struct Parameter {
	public:
		std::shared_ptr<Texture> texValue;
		float floatValue;
		DirectX::XMFLOAT4 vecValue;
	};

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
	std::map<std::string, Parameter> m_Params;
};

