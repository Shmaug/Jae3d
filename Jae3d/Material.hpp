#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <variant>
#include <memory>

#include <d3d12.h>
#include <DirectXMath.h>

class CommandList;
class Shader;
class Texture;
class ConstantBuffer;

#include "../Common/ShaderAsset.hpp"

class Material {
public:
	jstring mName;

	Material(jstring name, std::shared_ptr<Shader> shader);
	~Material();

	void SetShader(std::shared_ptr<Shader> shader, bool resetParameters = false);

	void SetInt(jstring param, int v);
	void SetInt2(jstring param, DirectX::XMINT2 v);
	void SetInt3(jstring param, DirectX::XMINT3 v);
	void SetInt4(jstring param, DirectX::XMINT4 v);

	void SetUInt(jstring param, unsigned int v);
	void SetUInt2(jstring param, DirectX::XMUINT2 v);
	void SetUInt3(jstring param, DirectX::XMUINT3 v);
	void SetUInt4(jstring param, DirectX::XMUINT4 v);

	void SetFloat(jstring param, float v);
	void SetFloat2(jstring param, DirectX::XMFLOAT2 vec);
	void SetFloat3(jstring param, DirectX::XMFLOAT3 vec);
	void SetFloat4(jstring param, DirectX::XMFLOAT4 vec);

	void SetColor3(jstring param, DirectX::XMFLOAT3 vec);
	void SetColor4(jstring param, DirectX::XMFLOAT4 vec);

	void SetTexture(jstring param, std::shared_ptr<Texture> tex);
	void SetCBuffer(jstring param, std::shared_ptr<ConstantBuffer> tex);

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
	void SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	// constant buffers for integral material parameters: [root index, cbuffer]
	int mParamCbufferCount;
	ParameterCBuffer* mParamCbuffers;

	std::shared_ptr<Shader> mShader;
	jmap<jstring, MaterialValue> mParamValues;
};