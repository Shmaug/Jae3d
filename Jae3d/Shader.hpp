#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <DirectXMath.h>

#include "Asset.hpp"
#include "Mesh.hpp"

#include "jvector.hpp"
#include "jmap.hpp"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class Shader : public Asset {
	public:enum SHADERSTAGE {
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

		jwstring ToString() {
			int c = 0;
			wchar_t* buf = new wchar_t[256];
			switch (type) {
			case PARAM_TYPE_CBUFFER:
				c += swprintf_s(buf, 256, L"Root CBV");
				break;
			case PARAM_TYPE_SRV:
				c += swprintf_s(buf, 256, L"Root SRV");
				break;
			case PARAM_TYPE_UAV:
				c += swprintf_s(buf, 256, L"Root UAV");
				break;
			case PARAM_TYPE_SAMPLER:
				c += swprintf_s(buf, 256, L"Root Sampler");
				break;
			case PARAM_TYPE_TEXTURE:
				c += swprintf_s(buf, 256, L"Root Texture");
				break;

			case PARAM_TYPE_FLOATRANGE:
				c += swprintf_s(buf, 256, L"Float: %f Range: (%f,%f)", defaultValue.floatValue, defaultValue.floatRange.x, defaultValue.floatRange.y);
				break;
			case PARAM_TYPE_INTRANGE:
				c += swprintf_s(buf, 256, L"Int: %d Range: (%d,%d)", defaultValue.intValue, defaultValue.intRange.x, defaultValue.intRange.y);
				break;
			case PARAM_TYPE_UINTRANGE:
				c += swprintf_s(buf, 256, L"UInt: %u Range: (%u,%u)", defaultValue.uintValue, defaultValue.uintRange.x, defaultValue.uintRange.y);
				break;

			case PARAM_TYPE_FLOAT:
				c += swprintf_s(buf, 256, L"Float:%f", defaultValue.floatValue);
				break;
			case PARAM_TYPE_FLOAT2:
				c += swprintf_s(buf, 256, L"Float2: %f %f", defaultValue.float2Value.x, defaultValue.float2Value.y);
				break;
			case PARAM_TYPE_COLOR3:
				c += swprintf_s(buf, 256, L"Color3: %f %f %f", defaultValue.float3Value.x, defaultValue.float3Value.y, defaultValue.float3Value.z);
				break;
			case PARAM_TYPE_FLOAT3:
				c += swprintf_s(buf, 256, L"Float3: %f %f %f", defaultValue.float3Value.x, defaultValue.float3Value.y, defaultValue.float3Value.z);
				break;
			case PARAM_TYPE_COLOR4:
				c += swprintf_s(buf, 256, L"Color4: %f %f %f %f", defaultValue.float4Value.x, defaultValue.float4Value.y, defaultValue.float4Value.z, defaultValue.float4Value.w);
				break;
			case PARAM_TYPE_FLOAT4:
				c += swprintf_s(buf, 256, L"Float4: %f %f %f %f", defaultValue.float4Value.x, defaultValue.float4Value.y, defaultValue.float4Value.z, defaultValue.float4Value.w);
				break;

			case PARAM_TYPE_INT:
				c += swprintf_s(buf, 256, L"Int: %d", defaultValue.intValue);
				break;
			case PARAM_TYPE_INT2:
				c += swprintf_s(buf, 256, L"Int2: %d %d", defaultValue.int2Value.x, defaultValue.int2Value.y);
				break;
			case PARAM_TYPE_INT3:
				c += swprintf_s(buf, 256, L"Int3: %d %d %d", defaultValue.int3Value.x, defaultValue.int3Value.y, defaultValue.int3Value.z);
				break;
			case PARAM_TYPE_INT4:
				c += swprintf_s(buf, 256, L"Int3: %d %d %d %d", defaultValue.int4Value.x, defaultValue.int4Value.y, defaultValue.int4Value.z, defaultValue.int4Value.w);
				break;

			case PARAM_TYPE_UINT:
				c += swprintf_s(buf, 256, L"UInt: %u", defaultValue.uintValue);
				break;
			case PARAM_TYPE_UINT2:
				c += swprintf_s(buf, 256, L"UInt2: %u %u", defaultValue.uint2Value.x, defaultValue.uint2Value.y);
				break;
			case PARAM_TYPE_UINT3:
				c += swprintf_s(buf, 256, L"UInt3: %u %u %u", defaultValue.uint3Value.x, defaultValue.uint3Value.y, defaultValue.uint3Value.z);
				break;
			case PARAM_TYPE_UINT4:
				c += swprintf_s(buf, 256, L"UInt3: %u %u %u %u", defaultValue.uint4Value.x, defaultValue.uint4Value.y, defaultValue.uint4Value.z, defaultValue.uint4Value.w);
				break;
			}

			jwstring str(buf);
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

	JAE_API Shader(jwstring name);
	JAE_API Shader(jwstring name, MemoryStream &ms);
	JAE_API ~Shader();

	JAE_API void Upload();

	JAE_API HRESULT ReadShaderStage(jwstring path, SHADERSTAGE stage);
	JAE_API HRESULT CompileShaderStage(jwstring path, jwstring entryPoint, SHADERSTAGE stage);
	JAE_API HRESULT CompileShaderStage(const char* text, const char* entryPoint, SHADERSTAGE stage);

	JAE_API void WriteData(MemoryStream &ms);
	JAE_API uint64_t TypeId();

	ShaderParameter* GetParameter(jwstring name) { return &mParams.at(name); }

	// Not to be used at runtime
	void AddParameter(jwstring name, ShaderParameter &param) { mParams.emplace(name, param); }
	// Not to be used at runtime
	void AddParameterBuffer(int rootIndex, int size) { mCBufferParameters.push_back(ParameterBuffer(rootIndex, size)); }
	int GetParameterBufferCount() const { return (int)mCBufferParameters.size(); }
	ParameterBuffer GetParameterBuffer(int index) const { return mCBufferParameters[index]; }

	bool HasParameters() const { return !mParams.empty(); }
	jmap<jwstring, ShaderParameter>::jmap_iterator ParameterBegin() { return mParams.begin(); }

private:
	friend class CommandList;
	JAE_API bool SetActive(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);
	JAE_API void SetPSO(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, Mesh::SEMANTIC input);
	JAE_API void SetCompute(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	jmap<Mesh::SEMANTIC, _WRL::ComPtr<ID3D12PipelineState>> mStates;
	_WRL::ComPtr<ID3D12PipelineState> mComputePSO;

	JAE_API _WRL::ComPtr<ID3D12PipelineState> CreatePSO(Mesh::SEMANTIC input);
	void CreateComputePSO();

	bool mCreated = false;
	_WRL::ComPtr<ID3D12RootSignature> mRootSignature;

	jvector<ParameterBuffer> mCBufferParameters;

	jmap<jwstring, ShaderParameter> mParams;
	ID3DBlob* mBlobs[7] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
};