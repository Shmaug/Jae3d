#pragma once

#include "Util.hpp"
#include "IOUtil.hpp"

#include "jstring.hpp"
#include "jmap.hpp"
#include "jvector.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include "d3dx12.hpp"

#include <variant>
#include <memory>


class Asset;
class Mesh;
class Shader;
class Texture;
class ConstantBuffer;
class CommandList;
class Camera;
class Material;

const D3D12_RENDER_TARGET_BLEND_DESC BLEND_STATE_DEFAULT {
	FALSE,							// BOOL BlendEnable;
	FALSE,							// BOOL LogicOpEnable;
	D3D12_BLEND_ONE,				// D3D12_BLEND SrcBlend;
	D3D12_BLEND_ZERO,				// D3D12_BLEND DestBlend;
	D3D12_BLEND_OP_ADD,				// D3D12_BLEND_OP BlendOp;
	D3D12_BLEND_ONE,				// D3D12_BLEND SrcBlendAlpha;
	D3D12_BLEND_ZERO,				// D3D12_BLEND DestBlendAlpha;
	D3D12_BLEND_OP_ADD,				// D3D12_BLEND_OP BlendOpAlpha;
	D3D12_LOGIC_OP_NOOP,			// D3D12_LOGIC_OP LogicOp;
	D3D12_COLOR_WRITE_ENABLE_ALL	// UINT8 RenderTargetWriteMask;
};
const D3D12_RENDER_TARGET_BLEND_DESC BLEND_STATE_ALPHA {
	TRUE,							// BOOL BlendEnable;
	FALSE,							// BOOL LogicOpEnable;
	D3D12_BLEND_SRC_ALPHA,			// D3D12_BLEND SrcBlend;
	D3D12_BLEND_INV_SRC_ALPHA,		// D3D12_BLEND DestBlend;
	D3D12_BLEND_OP_ADD,				// D3D12_BLEND_OP BlendOp;
	D3D12_BLEND_SRC_ALPHA,			// D3D12_BLEND SrcBlendAlpha;
	D3D12_BLEND_INV_SRC_ALPHA,		// D3D12_BLEND DestBlendAlpha;
	D3D12_BLEND_OP_ADD,				// D3D12_BLEND_OP BlendOpAlpha;
	D3D12_LOGIC_OP_NOOP,			// D3D12_LOGIC_OP LogicOp;
	D3D12_COLOR_WRITE_ENABLE_ALL	// UINT8 RenderTargetWriteMask;
};
const D3D12_RENDER_TARGET_BLEND_DESC BLEND_STATE_PREMUL {
	TRUE,							// BOOL BlendEnable;
	FALSE,							// BOOL LogicOpEnable;
	D3D12_BLEND_ONE,				// D3D12_BLEND SrcBlend;
	D3D12_BLEND_INV_SRC_ALPHA,		// D3D12_BLEND DestBlend;
	D3D12_BLEND_OP_ADD,				// D3D12_BLEND_OP BlendOp;
	D3D12_BLEND_SRC_ALPHA,			// D3D12_BLEND SrcBlendAlpha;
	D3D12_BLEND_INV_SRC_ALPHA,		// D3D12_BLEND DestBlendAlpha;
	D3D12_BLEND_OP_ADD,				// D3D12_BLEND_OP BlendOpAlpha;
	D3D12_LOGIC_OP_NOOP,			// D3D12_LOGIC_OP LogicOp;
	D3D12_COLOR_WRITE_ENABLE_ALL	// UINT8 RenderTargetWriteMask;
};

enum ASSET_TYPE : uint32_t {
	ASSET_TYPE_UNSPECIFIED	= 0,
	ASSET_TYPE_MESH			= 1,
	ASSET_TYPE_SHADER		= 2,
	ASSET_TYPE_TEXTURE		= 3,
	ASSET_TYPE_FONT			= 4,
};

enum MESH_SEMANTIC {
	MESH_SEMANTIC_POSITION		= 0x000, // always have position
	MESH_SEMANTIC_NORMAL		= 0x001,
	MESH_SEMANTIC_TANGENT		= 0x002,
	MESH_SEMANTIC_BINORMAL		= 0x004,
	MESH_SEMANTIC_COLOR0		= 0x008,
	MESH_SEMANTIC_COLOR1		= 0x010,
	MESH_SEMANTIC_BLENDINDICES	= 0x020,
	MESH_SEMANTIC_BLENDWEIGHT	= 0x040,
	MESH_SEMANTIC_TEXCOORD0		= 0x080,
	MESH_SEMANTIC_TEXCOORD1		= 0x100,
	MESH_SEMANTIC_TEXCOORD2		= 0x200,
	MESH_SEMANTIC_TEXCOORD3		= 0x400,
};

enum SHADER_STAGE {
	SHADER_STAGE_ROOTSIG,
	SHADER_STAGE_VERTEX,
	SHADER_STAGE_HULL,
	SHADER_STAGE_DOMAIN,
	SHADER_STAGE_GEOMETRY,
	SHADER_STAGE_PIXEL,
	SHADER_STAGE_COMPUTE
};
enum SHADER_PARAM_TYPE {
	SHADER_PARAM_TYPE_CBUFFER,
	SHADER_PARAM_TYPE_SRV,
	SHADER_PARAM_TYPE_UAV,
	SHADER_PARAM_TYPE_SAMPLER,
	SHADER_PARAM_TYPE_TEXTURE,
	
	SHADER_PARAM_TYPE_FLOATRANGE,
	SHADER_PARAM_TYPE_INTRANGE,
	SHADER_PARAM_TYPE_UINTRANGE,
	
	SHADER_PARAM_TYPE_COLOR3,
	SHADER_PARAM_TYPE_COLOR4,
	
	SHADER_PARAM_TYPE_FLOAT,
	SHADER_PARAM_TYPE_FLOAT2,
	SHADER_PARAM_TYPE_FLOAT3,
	SHADER_PARAM_TYPE_FLOAT4,
	
	SHADER_PARAM_TYPE_INT,
	SHADER_PARAM_TYPE_INT2,
	SHADER_PARAM_TYPE_INT3,
	SHADER_PARAM_TYPE_INT4,
	
	SHADER_PARAM_TYPE_UINT,
	SHADER_PARAM_TYPE_UINT2,
	SHADER_PARAM_TYPE_UINT3,
	SHADER_PARAM_TYPE_UINT4,
};

enum ALPHA_MODE {
	ALPHA_MODE_TRANSPARENCY = 0,
	ALPHA_MODE_PREMULTIPLIED = 1,
	ALPHA_MODE_OTHER = 2,
	ALPHA_MODE_UNUSED = 3,
};

// Stores all types of material parameters and their values
struct MaterialValue {
	union Range {
		DirectX::XMFLOAT2 floatRange;
		DirectX::XMINT2 intRange;
		DirectX::XMUINT2 uintRange;

		Range() : uintRange(DirectX::XMUINT2()) {};
		~Range() {};
	};
	using Value = std::variant<
		float,
		DirectX::XMFLOAT2,
		DirectX::XMFLOAT3,
		DirectX::XMFLOAT4,
		int,
		DirectX::XMINT2,
		DirectX::XMINT3,
		DirectX::XMINT4,
		unsigned int,
		DirectX::XMUINT2,
		DirectX::XMUINT3,
		DirectX::XMUINT4,
		std::shared_ptr<Texture>,
		std::shared_ptr<ConstantBuffer>>;

	Range range;
	Value value;
	int cbufferIndex;

	MaterialValue() : cbufferIndex(-1) { range.floatRange = DirectX::XMFLOAT2(); value = 0.0f; }
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
// Stores numeric material parameters in a cuffer at rootIndex
struct MaterialParameterCBuffer {
	int rootIndex;
	std::shared_ptr<ConstantBuffer> cbuffer;
	MaterialParameterCBuffer() : rootIndex(-1), cbuffer(nullptr) {}
	MaterialParameterCBuffer(int rootIndex, std::shared_ptr<ConstantBuffer> cbuffer) : rootIndex(rootIndex), cbuffer(cbuffer) {}
};
// Stores information about a shader parameter and a default value if it is a numeric type
struct ShaderParameter {
public:
	// Stores just numeric types for default values (set in shader)
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

	ShaderParameter() : type(SHADER_PARAM_TYPE_FLOAT), rootIndex(0), cbufferOffset(0) {}
	ShaderParameter(SHADER_PARAM_TYPE type, unsigned int rootIndex, unsigned int cbufferOffset, ShaderValue value)
		: type(type), rootIndex(rootIndex), cbufferOffset(cbufferOffset), defaultValue(value) {}
	SHADER_PARAM_TYPE Type() const { return type; }
	unsigned int RootIndex() const { return rootIndex; }
	unsigned int CBufferOffset() const { return cbufferOffset; }
	ShaderValue GetDefaultValue() const { return defaultValue; }

	jwstring ToString() {
		int c = 0;
		wchar_t* buf = new wchar_t[256];
		switch (type) {
		case SHADER_PARAM_TYPE_CBUFFER:
			c += swprintf_s(buf, 256, L"Root CBV");
			break;
		case SHADER_PARAM_TYPE_SRV:
			c += swprintf_s(buf, 256, L"Root SRV");
			break;
		case SHADER_PARAM_TYPE_UAV:
			c += swprintf_s(buf, 256, L"Root UAV");
			break;
		case SHADER_PARAM_TYPE_SAMPLER:
			c += swprintf_s(buf, 256, L"Root Sampler");
			break;
		case SHADER_PARAM_TYPE_TEXTURE:
			c += swprintf_s(buf, 256, L"Root Texture");
			break;

		case SHADER_PARAM_TYPE_FLOATRANGE:
			c += swprintf_s(buf, 256, L"Float: %f Range: (%f,%f)", defaultValue.floatValue, defaultValue.floatRange.x, defaultValue.floatRange.y);
			break;
		case SHADER_PARAM_TYPE_INTRANGE:
			c += swprintf_s(buf, 256, L"Int: %d Range: (%d,%d)", defaultValue.intValue, defaultValue.intRange.x, defaultValue.intRange.y);
			break;
		case SHADER_PARAM_TYPE_UINTRANGE:
			c += swprintf_s(buf, 256, L"UInt: %u Range: (%u,%u)", defaultValue.uintValue, defaultValue.uintRange.x, defaultValue.uintRange.y);
			break;

		case SHADER_PARAM_TYPE_FLOAT:
			c += swprintf_s(buf, 256, L"Float:%f", defaultValue.floatValue);
			break;
		case SHADER_PARAM_TYPE_FLOAT2:
			c += swprintf_s(buf, 256, L"Float2: %f %f", defaultValue.float2Value.x, defaultValue.float2Value.y);
			break;
		case SHADER_PARAM_TYPE_COLOR3:
			c += swprintf_s(buf, 256, L"Color3: %f %f %f", defaultValue.float3Value.x, defaultValue.float3Value.y, defaultValue.float3Value.z);
			break;
		case SHADER_PARAM_TYPE_FLOAT3:
			c += swprintf_s(buf, 256, L"Float3: %f %f %f", defaultValue.float3Value.x, defaultValue.float3Value.y, defaultValue.float3Value.z);
			break;
		case SHADER_PARAM_TYPE_COLOR4:
			c += swprintf_s(buf, 256, L"Color4: %f %f %f %f", defaultValue.float4Value.x, defaultValue.float4Value.y, defaultValue.float4Value.z, defaultValue.float4Value.w);
			break;
		case SHADER_PARAM_TYPE_FLOAT4:
			c += swprintf_s(buf, 256, L"Float4: %f %f %f %f", defaultValue.float4Value.x, defaultValue.float4Value.y, defaultValue.float4Value.z, defaultValue.float4Value.w);
			break;

		case SHADER_PARAM_TYPE_INT:
			c += swprintf_s(buf, 256, L"Int: %d", defaultValue.intValue);
			break;
		case SHADER_PARAM_TYPE_INT2:
			c += swprintf_s(buf, 256, L"Int2: %d %d", defaultValue.int2Value.x, defaultValue.int2Value.y);
			break;
		case SHADER_PARAM_TYPE_INT3:
			c += swprintf_s(buf, 256, L"Int3: %d %d %d", defaultValue.int3Value.x, defaultValue.int3Value.y, defaultValue.int3Value.z);
			break;
		case SHADER_PARAM_TYPE_INT4:
			c += swprintf_s(buf, 256, L"Int3: %d %d %d %d", defaultValue.int4Value.x, defaultValue.int4Value.y, defaultValue.int4Value.z, defaultValue.int4Value.w);
			break;

		case SHADER_PARAM_TYPE_UINT:
			c += swprintf_s(buf, 256, L"UInt: %u", defaultValue.uintValue);
			break;
		case SHADER_PARAM_TYPE_UINT2:
			c += swprintf_s(buf, 256, L"UInt2: %u %u", defaultValue.uint2Value.x, defaultValue.uint2Value.y);
			break;
		case SHADER_PARAM_TYPE_UINT3:
			c += swprintf_s(buf, 256, L"UInt3: %u %u %u", defaultValue.uint3Value.x, defaultValue.uint3Value.y, defaultValue.uint3Value.z);
			break;
		case SHADER_PARAM_TYPE_UINT4:
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
	SHADER_PARAM_TYPE type;
	unsigned int rootIndex;
	// for numeric parameters stored in a cbuffer
	unsigned int cbufferOffset;
	// for numeric types
	ShaderValue defaultValue; 
};
struct ShaderParameterBuffer {
	int rootIndex;
	int size;
	ShaderParameterBuffer() : rootIndex(-1), size(0) {}
	ShaderParameterBuffer(int rootIndex, int size) : rootIndex(rootIndex), size(size) {}
};

struct MeshBone {
	jwstring mName;
	DirectX::XMFLOAT4X4 mBoneToMesh;

	MeshBone() : mName(L""), mBoneToMesh(DirectX::XMFLOAT4X4()) {}
	MeshBone(const MeshBone &b) : mName(b.mName), mBoneToMesh(b.mBoneToMesh) {}
	MeshBone(jwstring name, DirectX::XMFLOAT4X4 m) : mName(name), mBoneToMesh(m) {}

	MeshBone& operator=(const MeshBone &rhs) {
		if (&rhs == this) return *this;
		mName = rhs.mName;
		mBoneToMesh = rhs.mBoneToMesh;
		return *this;
	}
};

struct FontGlyph {
	wchar_t character;
	unsigned int advance;
	int ox;
	int oy;
	// texture rect
	unsigned int tx0;
	unsigned int ty0;
	unsigned int tw;
	unsigned int th;

	FontGlyph() : character(L'\0') {};
	~FontGlyph() {};
};
struct FontKerning {
	wchar_t from;
	wchar_t to;
	int offset;
};

struct ShaderState {
	MESH_SEMANTIC input;
	D3D12_RENDER_TARGET_BLEND_DESC blendState;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;
	D3D12_FILL_MODE fillMode;
	bool ztest;
	bool zwrite;

	ShaderState() : input(MESH_SEMANTIC_POSITION), blendState(BLEND_STATE_DEFAULT), topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE), ztest(true), zwrite(true), fillMode(D3D12_FILL_MODE_SOLID) {}
	ShaderState(const ShaderState &s) : input(s.input), blendState(s.blendState), topology(s.topology), ztest(s.ztest), zwrite(s.zwrite), fillMode(s.fillMode) {}
	~ShaderState() {}

	ShaderState& operator =(const ShaderState &rhs) {
		input = rhs.input;
		blendState = rhs.blendState;
		topology = rhs.topology;
		ztest = rhs.ztest;
		zwrite = rhs.zwrite;
		fillMode = rhs.fillMode;
		return *this;
	}
	bool operator ==(const ShaderState &rhs) {
		return input == rhs.input &&
			topology == rhs.topology &&
			ztest == rhs.ztest &&
			zwrite == rhs.zwrite && 
			fillMode == rhs.fillMode &&
			blendState.BlendEnable == rhs.blendState.BlendEnable &&
			blendState.BlendOp == rhs.blendState.BlendOp &&
			blendState.BlendOpAlpha == rhs.blendState.BlendOpAlpha &&
			blendState.DestBlend == rhs.blendState.DestBlend &&
			blendState.DestBlendAlpha == rhs.blendState.DestBlendAlpha &&
			blendState.LogicOp == rhs.blendState.LogicOp &&
			blendState.LogicOpEnable == rhs.blendState.LogicOpEnable &&
			blendState.RenderTargetWriteMask == rhs.blendState.RenderTargetWriteMask &&
			blendState.SrcBlend == rhs.blendState.SrcBlend &&
			blendState.SrcBlendAlpha == rhs.blendState.SrcBlendAlpha;
	}
};

template <class T>
inline void hash_combine(std::size_t &s, const T &v) {
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

namespace std {
	template<>
	struct hash<ShaderState> {
		size_t operator()(const ShaderState &s) const noexcept {
			size_t h = 0;
			hash_combine(h, s.input);
			hash_combine(h, s.topology);
			hash_combine(h, s.ztest);
			hash_combine(h, s.zwrite);
			hash_combine(h, s.fillMode);
			hash_combine(h, s.blendState.BlendEnable);
			hash_combine(h, s.blendState.BlendOp);
			hash_combine(h, s.blendState.BlendOpAlpha);
			hash_combine(h, s.blendState.DestBlend);
			hash_combine(h, s.blendState.DestBlendAlpha);
			hash_combine(h, s.blendState.LogicOp);
			hash_combine(h, s.blendState.LogicOpEnable);
			hash_combine(h, s.blendState.RenderTargetWriteMask);
			hash_combine(h, s.blendState.SrcBlend);
			hash_combine(h, s.blendState.SrcBlendAlpha);
			return h;
		}
	};
}