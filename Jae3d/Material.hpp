#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <variant>
#include <memory>

#include <d3d12.h>
#include <DirectXMath.h>

#include "jstring.hpp"
#include "jmap.hpp"

class CommandList;
class Shader;
class Texture;
class ConstantBuffer;

class Material {
public:
	jwstring mName;

	JAE_API Material(jwstring name, std::shared_ptr<Shader> shader);
	JAE_API ~Material();

	JAE_API void SetShader(std::shared_ptr<Shader> shader, bool resetParameters = false);

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

private:
	// Stores all types of material parameters
	struct MaterialValue {
		// Range
		union Range {
			DirectX::XMFLOAT2 floatRange;
			DirectX::XMINT2 intRange;
			DirectX::XMUINT2 uintRange;

			Range() : uintRange(DirectX::XMUINT2()) {};
			~Range() {};
		} ;
		// Value
		union Value {
			float floatValue;
			DirectX::XMFLOAT2 float2Value;
			DirectX::XMFLOAT3 float3Value;
			DirectX::XMFLOAT4 float4Value;
			int intValue;
			DirectX::XMINT2 int2Value;
			DirectX::XMINT3 int3Value;
			DirectX::XMINT4 int4Value;
			unsigned int uintValue;
			DirectX::XMUINT2 uint2Value;
			DirectX::XMUINT3 uint3Value;
			DirectX::XMUINT4 uint4Value;

			std::shared_ptr<Texture> textureValue;
			std::shared_ptr<ConstantBuffer> cbufferValue;

			Value() : uint4Value(DirectX::XMUINT4()) {};
			~Value() {};
		};
		Range range;
		Value value;
		int cbufferIndex;

		MaterialValue() : cbufferIndex(-1) { range.floatRange = DirectX::XMFLOAT2(); value.floatValue = 0.0f; }
		MaterialValue(const MaterialValue &mv) : cbufferIndex(mv.cbufferIndex) {
			memcpy(&range, &mv.range, sizeof(Range));
			memcpy(&value, &mv.value, sizeof(Value));
		}
		~MaterialValue() {}

		MaterialValue& operator=(const MaterialValue &rhs) {
			if (&rhs == this) return *this;
			memcpy(&range, &rhs.range, sizeof(Range));
			memcpy(&value, &rhs.value, sizeof(Value));
			cbufferIndex = rhs.cbufferIndex;
			return *this;
		}
	};
	struct ParameterCBuffer {
		int rootIndex;
		std::shared_ptr<ConstantBuffer> cbuffer;
		ParameterCBuffer() : rootIndex(-1), cbuffer(nullptr) {}
		ParameterCBuffer(int rootIndex, std::shared_ptr<ConstantBuffer> cbuffer) : rootIndex(rootIndex), cbuffer(cbuffer) {}
	};

	friend class CommandList;
	JAE_API void SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, unsigned int frameIndex);

	// constant buffers for integral material parameters: [root index, cbuffer]
	int mParamCbufferCount;
	ParameterCBuffer* mParamCbuffers;

	std::shared_ptr<Shader> mShader;
	jmap<jwstring, MaterialValue> mParamValues;
};