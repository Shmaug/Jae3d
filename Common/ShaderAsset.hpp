#pragma once

#include "../Jae3d/Util.hpp"
#include "jvector.hpp"
#include "jstring.hpp"
#include "jmap.hpp"

#include "Asset.hpp"

#include <variant>
#include <d3d12.h>

#include <d3d12.h>
#include <DirectXMath.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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
		PARAM_TYPE_CBUFFER,
		PARAM_TYPE_SRV,
		PARAM_TYPE_UAV,
		PARAM_TYPE_SAMPLER,
		PARAM_TYPE_TEXTURE,

		PARAM_TYPE_FLOATRANGE,
		PARAM_TYPE_INTRANGE,
		PARAM_TYPE_UINTRANGE,

		PARAM_TYPE_COLOR3,
		PARAM_TYPE_COLOR4,

		PARAM_TYPE_FLOAT,
		PARAM_TYPE_FLOAT2,
		PARAM_TYPE_FLOAT3,
		PARAM_TYPE_FLOAT4,

		PARAM_TYPE_INT,
		PARAM_TYPE_INT2,
		PARAM_TYPE_INT3,
		PARAM_TYPE_INT4,

		PARAM_TYPE_UINT,
		PARAM_TYPE_UINT2,
		PARAM_TYPE_UINT3,
		PARAM_TYPE_UINT4,
	};

	struct ShaderParameter {
	public:
		// Stores just integral types
		struct ShaderValue {
			union {
				DirectX::XMFLOAT2 floatRange;
				DirectX::XMINT2 intRange;
				DirectX::XMUINT2 uintRange;
			};
			union {
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
			};

			ShaderValue() {}
			ShaderValue(float v) : floatValue(v), floatRange(DirectX::XMFLOAT2(0, 0)) {}
			~ShaderValue() {}

			ShaderValue& operator=(const ShaderValue &rhs) {
				memcpy(this, &rhs, sizeof(ShaderValue));
				return *this;
			}
		};

		ShaderParameter() : type(PARAM_TYPE_FLOAT), rootIndex(0), cbufferOffset(0) {}
		ShaderParameter(PARAM_TYPE type, unsigned int rootIndex, unsigned int cbufferOffset, ShaderValue value)
			: type(type), rootIndex(rootIndex), cbufferOffset(cbufferOffset), defaultValue(value) {}
		PARAM_TYPE Type() const { return type; }
		unsigned int RootIndex() const { return rootIndex; }
		unsigned int CBufferOffset() const { return cbufferOffset; }
		ShaderValue GetDefaultValue() const { return defaultValue; }

		jstring ToString(){
			int c = 0;
			char* buf = new char[256];
			switch (type) {
			case ShaderAsset::PARAM_TYPE_CBUFFER:
				c += sprintf_s(buf, 256, "Root CBV");
				break;
			case ShaderAsset::PARAM_TYPE_SRV:
				c += sprintf_s(buf, 256, "Root SRV");
				break;
			case ShaderAsset::PARAM_TYPE_UAV:
				c += sprintf_s(buf, 256, "Root UAV");
				break;
			case ShaderAsset::PARAM_TYPE_SAMPLER:
				c += sprintf_s(buf, 256, "Root Sampler");
				break;
			case ShaderAsset::PARAM_TYPE_TEXTURE:
				c += sprintf_s(buf, 256, "Root Texture");
				break;

			case ShaderAsset::PARAM_TYPE_FLOATRANGE:
				c += sprintf_s(buf, 256, "Float: %f Range: (%f,%f)", defaultValue.floatValue, defaultValue.floatRange.x, defaultValue.floatRange.y);
				break;
			case ShaderAsset::PARAM_TYPE_INTRANGE:
				c += sprintf_s(buf, 256, "Int: %d Range: (%d,%d)", defaultValue.intValue, defaultValue.intRange.x, defaultValue.intRange.y);
				break;
			case ShaderAsset::PARAM_TYPE_UINTRANGE:
				c += sprintf_s(buf, 256, "UInt: %u Range: (%u,%u)", defaultValue.uintValue, defaultValue.uintRange.x, defaultValue.uintRange.y);
				break;

			case ShaderAsset::PARAM_TYPE_FLOAT:
				c += sprintf_s(buf, 256, "Float:%f", defaultValue.floatValue);
				break;
			case ShaderAsset::PARAM_TYPE_FLOAT2:
				c += sprintf_s(buf, 256, "Float2: %f %f", defaultValue.float2Value.x, defaultValue.float2Value.y);
				break;
			case ShaderAsset::PARAM_TYPE_COLOR3:
				c += sprintf_s(buf, 256, "Color3: %f %f %f", defaultValue.float3Value.x, defaultValue.float3Value.y, defaultValue.float3Value.z);
				break;
			case ShaderAsset::PARAM_TYPE_FLOAT3:
				c += sprintf_s(buf, 256, "Float3: %f %f %f", defaultValue.float3Value.x, defaultValue.float3Value.y, defaultValue.float3Value.z);
				break;
			case ShaderAsset::PARAM_TYPE_COLOR4:
				c += sprintf_s(buf, 256, "Color4: %f %f %f %f", defaultValue.float4Value.x, defaultValue.float4Value.y, defaultValue.float4Value.z, defaultValue.float4Value.w);
				break;
			case ShaderAsset::PARAM_TYPE_FLOAT4:
				c += sprintf_s(buf, 256, "Float4: %f %f %f %f", defaultValue.float4Value.x, defaultValue.float4Value.y, defaultValue.float4Value.z, defaultValue.float4Value.w);
				break;

			case ShaderAsset::PARAM_TYPE_INT:
				c += sprintf_s(buf, 256, "Int: %d", defaultValue.intValue);
				break;
			case ShaderAsset::PARAM_TYPE_INT2:
				c += sprintf_s(buf, 256, "Int2: %d %d", defaultValue.int2Value.x, defaultValue.int2Value.y);
				break;
			case ShaderAsset::PARAM_TYPE_INT3:
				c += sprintf_s(buf, 256, "Int3: %d %d %d", defaultValue.int3Value.x, defaultValue.int3Value.y, defaultValue.int3Value.z);
				break;
			case ShaderAsset::PARAM_TYPE_INT4:
				c += sprintf_s(buf, 256, "Int3: %d %d %d %d", defaultValue.int4Value.x, defaultValue.int4Value.y, defaultValue.int4Value.z, defaultValue.int4Value.w);
				break;

			case ShaderAsset::PARAM_TYPE_UINT:
				c += sprintf_s(buf, 256, "UInt: %u", defaultValue.uintValue);
				break;
			case ShaderAsset::PARAM_TYPE_UINT2:
				c += sprintf_s(buf, 256, "UInt2: %u %u", defaultValue.uint2Value.x, defaultValue.uint2Value.y);
				break;
			case ShaderAsset::PARAM_TYPE_UINT3:
				c += sprintf_s(buf, 256, "UInt3: %u %u %u", defaultValue.uint3Value.x, defaultValue.uint3Value.y, defaultValue.uint3Value.z);
				break;
			case ShaderAsset::PARAM_TYPE_UINT4:
				c += sprintf_s(buf, 256, "UInt3: %u %u %u %u", defaultValue.uint4Value.x, defaultValue.uint4Value.y, defaultValue.uint4Value.z, defaultValue.uint4Value.w);
				break;
			}
			
			jstring str(buf);
			delete[] buf;
			return str;
		}

		ShaderParameter& operator =(const ShaderParameter &rhs) {
			type = rhs.type;
			rootIndex = rhs.rootIndex;
			cbufferOffset = rhs.cbufferOffset;
			defaultValue = rhs.defaultValue;
			return *this;
		}
	private:
		PARAM_TYPE type;
		unsigned int rootIndex;
		unsigned int cbufferOffset; // for integral parameters stored in a cbuffer
		ShaderValue defaultValue; // for integral types
	};
	struct ParameterBuffer {
		int rootIndex;
		int size;
		ParameterBuffer() : rootIndex(-1), size(0) {}
		ParameterBuffer(int rootIndex, int size) : rootIndex(rootIndex), size(size) {}
	};

	ShaderAsset(jstring name);
	ShaderAsset(jstring name, MemoryStream &ms);
	~ShaderAsset();

	HRESULT ReadShaderStage(jwstring path, SHADERSTAGE stage);
	HRESULT CompileShaderStage(jwstring path, jstring entryPoint, SHADERSTAGE stage);

	void WriteData(MemoryStream &ms);
	uint64_t TypeId();

	ID3DBlob* GetBlob(SHADERSTAGE stage) { return mBlobs[stage]; }

	ShaderParameter* GetParameter(jstring name) { return &mParams.at(name); }

	// Not to be used at runtime
	void AddParameter(jstring name, ShaderParameter &param) { mParams.emplace(name, param); }
	// Not to be used at runtime
	void AddParameterBuffer(int rootIndex, int size) { mCBufferParameters.push_back(ParameterBuffer(rootIndex, size)); }
	int GetParameterBufferCount() const { return (int)mCBufferParameters.size(); }
	ParameterBuffer GetParameterBuffer(int index) const { return mCBufferParameters[index]; }

	bool HasParameters() const { return !mParams.empty(); }
	jmap<jstring, ShaderParameter>::jmap_iterator ParameterBegin() { return mParams.begin(); }
	jmap<jstring, ShaderParameter>::jmap_iterator ParameterEnd() { return mParams.end(); }

private:
	jvector<ParameterBuffer> mCBufferParameters;

	jmap<jstring, ShaderParameter> mParams;
	ID3DBlob* mBlobs[7] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
};