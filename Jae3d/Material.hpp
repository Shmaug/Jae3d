#pragma once

#include "Common.hpp"
#include "jvector.hpp"

class Material {
public:
	jwstring mName;

	JAE_API Material(jwstring name, std::shared_ptr<Shader> shader);
	JAE_API ~Material();

	// Set shader and synchronize parameters
	JAE_API void SetShader(std::shared_ptr<Shader> shader, bool resetParameters = false);

	JAE_API bool IsKeywordEnabled(jstring keyword);
	JAE_API void EnableKeyword(jstring keyword);
	JAE_API void DisableKeyword(jstring keyword);
	JAE_API void SetKeywords(jvector<jstring> &keywords);

	JAE_API void SetInt(jwstring param, int v, unsigned int frameIndex);
	JAE_API void SetInt2(jwstring param, DirectX::XMINT2 v, unsigned int frameIndex);
	JAE_API void SetInt3(jwstring param, DirectX::XMINT3 v, unsigned int frameIndex);
	JAE_API void SetInt4(jwstring param, DirectX::XMINT4 v, unsigned int frameIndex);

	JAE_API void SetUInt(jwstring param, unsigned int v, unsigned int frameIndex);
	JAE_API void SetUInt2(jwstring param, DirectX::XMUINT2 v, unsigned int frameIndex);
	JAE_API void SetUInt3(jwstring param, DirectX::XMUINT3 v, unsigned int frameIndex);
	JAE_API void SetUInt4(jwstring param, DirectX::XMUINT4 v, unsigned int frameIndex);

	JAE_API void SetFloat(jwstring param, float v, unsigned int frameIndex);
	JAE_API void SetFloat2(jwstring param, DirectX::XMFLOAT2 vec, unsigned int frameIndex);
	JAE_API void SetFloat3(jwstring param, DirectX::XMFLOAT3 vec, unsigned int frameIndex);
	JAE_API void SetFloat4(jwstring param, DirectX::XMFLOAT4 vec, unsigned int frameIndex);

	JAE_API void SetColor3(jwstring param, DirectX::XMFLOAT3 vec, unsigned int frameIndex);
	JAE_API void SetColor4(jwstring param, DirectX::XMFLOAT4 vec, unsigned int frameIndex);

	JAE_API void SetTexture(jwstring param, std::shared_ptr<Texture> tex, unsigned int frameIndex);
	JAE_API void SetCBuffer(jwstring param, std::shared_ptr<ConstantBuffer> tex, unsigned int frameIndex);
	JAE_API void SetDescriptorTable(jwstring param, std::shared_ptr<DescriptorTable> tbl, unsigned int frameIndex);

private:
	jvector<jstring> mKeywords;

	jvector<CommandList*> mActive;
	friend class CommandList;
	// Sets the shader and all parameters
	JAE_API void SetActive(CommandList* commandList);

	// constant buffers for integral material parameters: [root index, cbuffer]
	int mParamCbufferCount;
	MaterialParameterCBuffer* mParamCbuffers;

	std::shared_ptr<Shader> mShader;
	std::unordered_map<jwstring, MaterialValue> mParamValues;
};