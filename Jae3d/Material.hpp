#pragma once

#include <set>

#include "Common.hpp"
#include "jvector.hpp"

class Material {
public:
	jwstring mName;

	JAE_API Material(const jwstring& name, const std::shared_ptr<Shader>& shader);
	JAE_API ~Material();

	// Set shader and synchronize parameters
	JAE_API void SetShader(const std::shared_ptr<Shader>& shader, bool resetParameters = false);
	inline std::shared_ptr<Shader> GetShader() const { return mShader; }

	JAE_API bool IsKeywordEnabled(const char* keyword);
	JAE_API const jvector<jstring>& GetKeywords() const { return mKeywords; }
	JAE_API void EnableKeyword(const char* keyword);
	JAE_API void DisableKeyword(const char* keyword);

	JAE_API void SetInt (const char* param, const int& v, unsigned int frameIndex);
	JAE_API void SetInt2(const char* param, const DirectX::XMINT2& v, unsigned int frameIndex);
	JAE_API void SetInt3(const char* param, const DirectX::XMINT3& v, unsigned int frameIndex);
	JAE_API void SetInt4(const char* param, const DirectX::XMINT4& v, unsigned int frameIndex);

	JAE_API void SetUInt (const char* param, const unsigned int& v, unsigned int frameIndex);
	JAE_API void SetUInt2(const char* param, const DirectX::XMUINT2& v, unsigned int frameIndex);
	JAE_API void SetUInt3(const char* param, const DirectX::XMUINT3& v, unsigned int frameIndex);
	JAE_API void SetUInt4(const char* param, const DirectX::XMUINT4& v, unsigned int frameIndex);

	JAE_API void SetFloat (const char* param, const float& v, unsigned int frameIndex);
	JAE_API void SetFloat2(const char* param, const DirectX::XMFLOAT2& vec, unsigned int frameIndex);
	JAE_API void SetFloat3(const char* param, const DirectX::XMFLOAT3& vec, unsigned int frameIndex);
	JAE_API void SetFloat4(const char* param, const DirectX::XMFLOAT4& vec, unsigned int frameIndex);

	JAE_API void SetColor3(const char* param, const DirectX::XMFLOAT3& vec, unsigned int frameIndex);
	JAE_API void SetColor4(const char* param, const DirectX::XMFLOAT4& vec, unsigned int frameIndex);

	JAE_API void SetTexture(const char* param, const std::shared_ptr<Texture>& tex, unsigned int frameIndex);
	JAE_API void SetCBuffer(const char* param, const std::shared_ptr<ConstantBuffer>& tex, unsigned int frameIndex);
	JAE_API void SetDescriptorTable(const char* param, const std::shared_ptr<DescriptorTable>& tbl, unsigned int frameIndex);

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

	bool Equals(const Material& m, unsigned int frameIndex) const;

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