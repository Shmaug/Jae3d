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

	JAE_API void SetInt(jstring param, int v, unsigned int frameIndex);
	JAE_API void SetInt2(jstring param, DirectX::XMINT2 v, unsigned int frameIndex);
	JAE_API void SetInt3(jstring param, DirectX::XMINT3 v, unsigned int frameIndex);
	JAE_API void SetInt4(jstring param, DirectX::XMINT4 v, unsigned int frameIndex);

	JAE_API void SetUInt(jstring param, unsigned int v, unsigned int frameIndex);
	JAE_API void SetUInt2(jstring param, DirectX::XMUINT2 v, unsigned int frameIndex);
	JAE_API void SetUInt3(jstring param, DirectX::XMUINT3 v, unsigned int frameIndex);
	JAE_API void SetUInt4(jstring param, DirectX::XMUINT4 v, unsigned int frameIndex);

	JAE_API void SetFloat(jstring param, float v, unsigned int frameIndex);
	JAE_API void SetFloat2(jstring param, DirectX::XMFLOAT2 vec, unsigned int frameIndex);
	JAE_API void SetFloat3(jstring param, DirectX::XMFLOAT3 vec, unsigned int frameIndex);
	JAE_API void SetFloat4(jstring param, DirectX::XMFLOAT4 vec, unsigned int frameIndex);

	JAE_API void SetColor3(jstring param, DirectX::XMFLOAT3 vec, unsigned int frameIndex);
	JAE_API void SetColor4(jstring param, DirectX::XMFLOAT4 vec, unsigned int frameIndex);

	JAE_API void SetTexture(jstring param, std::shared_ptr<Texture> tex, unsigned int frameIndex);
	JAE_API void SetCBuffer(jstring param, std::shared_ptr<ConstantBuffer> tex, unsigned int frameIndex);
	JAE_API void SetDescriptorTable(jstring param, std::shared_ptr<DescriptorTable> tbl, unsigned int frameIndex);

	inline void RenderQueue(unsigned int x) { mRenderQueue = x; }
	inline unsigned int RenderQueue() const { return mRenderQueue; }

	inline D3D12_CULL_MODE CullMode() const { return mCullMode; }
	inline void CullMode(D3D12_CULL_MODE c) { mCullMode = c; }
	inline bool ZWrite() const { return mZWrite; }
	inline void ZWrite(bool x) { mZWrite = x; }
	inline bool ZTest() const { return mZTest; }
	inline void ZTest(bool x) { mZTest = x; }
	inline D3D12_RENDER_TARGET_BLEND_DESC Blend() const { return mBlend; }
	inline void Blend(D3D12_RENDER_TARGET_BLEND_DESC x) { mBlend = x; }

private:
	jvector<jstring> mKeywords;

	friend class CommandList;
	// Sets the shader and all parameters (constant buffers/textures/etc) on the GPU
	JAE_API void SetActive(CommandList* commandList);

	// constant buffers for integral material parameters: [root index, cbuffer]
	int mParamCbufferCount;
	MaterialParameterCBuffer* mParamCbuffers;
	unsigned int mRenderQueue;
	bool mZWrite;
	bool mZTest;
	D3D12_CULL_MODE mCullMode;
	D3D12_RENDER_TARGET_BLEND_DESC mBlend;

	std::shared_ptr<Shader> mShader;
	std::unordered_map<jstring, MaterialValue> mParamValues;
};