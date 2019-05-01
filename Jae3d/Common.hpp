#pragma once

#include "Util.hpp"
#include "IOUtil.hpp"

#include "jstring.hpp"
#include "jvector.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include "d3dx12.hpp"

#include <variant>
#include <memory>
#include <algorithm>

class Asset;
class Mesh;
class Shader;
class Texture;
class Animation;
class Model;
class ConstantBuffer;
class DescriptorTable;
class CommandList;
class Camera;
class Material;

// BOOL BlendEnable;
// BOOL LogicOpEnable;
// D3D12_BLEND SrcBlend;
// D3D12_BLEND DestBlend;
// D3D12_BLEND_OP BlendOp;
// D3D12_BLEND SrcBlendAlpha;
// D3D12_BLEND DestBlendAlpha;
// D3D12_BLEND_OP BlendOpAlpha;
// D3D12_LOGIC_OP LogicOp;
// UINT8 RenderTargetWriteMask;

#define BLEND_STATE_DEFAULT { \
	FALSE,							\
	FALSE,							\
	D3D12_BLEND_ONE,				\
	D3D12_BLEND_ZERO,				\
	D3D12_BLEND_OP_ADD,				\
	D3D12_BLEND_ONE,				\
	D3D12_BLEND_ZERO,				\
	D3D12_BLEND_OP_ADD,				\
	D3D12_LOGIC_OP_NOOP,			\
	D3D12_COLOR_WRITE_ENABLE_ALL	\
}
#define BLEND_STATE_ALPHA { \
	TRUE,							\
	FALSE,							\
	D3D12_BLEND_SRC_ALPHA,			\
	D3D12_BLEND_INV_SRC_ALPHA,		\
	D3D12_BLEND_OP_ADD,				\
	D3D12_BLEND_SRC_ALPHA,			\
	D3D12_BLEND_INV_SRC_ALPHA,		\
	D3D12_BLEND_OP_ADD,				\
	D3D12_LOGIC_OP_NOOP,			\
	D3D12_COLOR_WRITE_ENABLE_ALL	\
}
#define BLEND_STATE_PREMUL { \
	TRUE,							\
	FALSE,							\
	D3D12_BLEND_ONE,				\
	D3D12_BLEND_INV_SRC_ALPHA,		\
	D3D12_BLEND_OP_ADD,				\
	D3D12_BLEND_ONE,				\
	D3D12_BLEND_INV_SRC_ALPHA,		\
	D3D12_BLEND_OP_ADD,				\
	D3D12_LOGIC_OP_NOOP,			\
	D3D12_COLOR_WRITE_ENABLE_ALL	\
}

inline bool operator ==(const D3D12_RENDER_TARGET_BLEND_DESC& a, const D3D12_RENDER_TARGET_BLEND_DESC& b) {
	return
		a.BlendEnable == b.BlendEnable &&
		(!a.BlendEnable || (
			a.BlendOp == b.BlendOp &&
			a.BlendOpAlpha == b.BlendOpAlpha &&
			a.DestBlend == b.DestBlend &&
			a.DestBlendAlpha == b.DestBlendAlpha &&
			a.LogicOpEnable == b.LogicOpEnable &&
			(!a.LogicOpEnable || a.LogicOp == b.LogicOp) &&
			a.RenderTargetWriteMask == b.RenderTargetWriteMask &&
			a.SrcBlend == b.SrcBlend &&
			a.SrcBlendAlpha == b.SrcBlendAlpha
			));
}
inline bool operator !=(const D3D12_RENDER_TARGET_BLEND_DESC & a, const D3D12_RENDER_TARGET_BLEND_DESC & b) { return !(a == b); }

enum ASSET_TYPE : uint32_t {
	ASSET_TYPE_UNSPECIFIED	= 0,
	ASSET_TYPE_MESH			= 1,
	ASSET_TYPE_SHADER		= 2,
	ASSET_TYPE_TEXTURE		= 3,
	ASSET_TYPE_FONT			= 4,
	ASSET_TYPE_ANIMATION    = 5,
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
	SHADER_PARAM_TYPE_TABLE,
	
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

// Stores all types of material parameters and their values
class MaterialValue {
public:
	union Range {
		DirectX::XMFLOAT2 floatRange;
		DirectX::XMINT2 intRange;
		DirectX::XMUINT2 uintRange;

		Range() : uintRange(DirectX::XMUINT2()) {};
		~Range() {};
	};
	Range range;

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
	std::shared_ptr<DescriptorTable> tableValue;

	int cbufferIndex;
	int tableSize;

	JAE_API MaterialValue();
	JAE_API MaterialValue(const MaterialValue &mv);
	~MaterialValue() {}

	JAE_API void set(float val);
	JAE_API void set(DirectX::XMFLOAT2 val);
	JAE_API void set(DirectX::XMFLOAT3 val);
	JAE_API void set(DirectX::XMFLOAT4 val);
	JAE_API void set(int val);
	JAE_API void set(DirectX::XMINT2 val);
	JAE_API void set(DirectX::XMINT3 val);
	JAE_API void set(DirectX::XMINT4 val);
	JAE_API void set(unsigned int val);
	JAE_API void set(DirectX::XMUINT2 val);
	JAE_API void set(DirectX::XMUINT3 val);
	JAE_API void set(DirectX::XMUINT4 val);
	JAE_API void set(const std::shared_ptr<Texture>& val);
	JAE_API void set(const std::shared_ptr<ConstantBuffer>& val);
	JAE_API void set(const std::shared_ptr<DescriptorTable>& val);
	 
	JAE_API MaterialValue& operator=(const MaterialValue &rhs);
};
// Stores numeric material parameters in a cuffer at rootIndex
struct MaterialParameterCBuffer {
	int rootIndex;
	std::shared_ptr<ConstantBuffer> cbuffer;
	MaterialParameterCBuffer() : rootIndex(-1), cbuffer(nullptr) {}
	MaterialParameterCBuffer(int rootIndex, const std::shared_ptr<ConstantBuffer>& cbuffer) : rootIndex(rootIndex), cbuffer(cbuffer) {}
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

		ShaderValue() : floatValue(0), floatRange(DirectX::XMFLOAT2(0, 0)) {}
		ShaderValue(float v) : floatValue(v), floatRange(DirectX::XMFLOAT2(0, 0)) {}
		~ShaderValue() {}

		ShaderValue& operator=(const ShaderValue &rhs) {
			memcpy(this, &rhs, sizeof(ShaderValue));
			return *this;
		}
	};

	ShaderParameter() : type(SHADER_PARAM_TYPE_FLOAT), rootIndex(0), cbufferOffset(-1), tableSize(0) {}

	ShaderParameter(SHADER_PARAM_TYPE type, unsigned int rootIndex, unsigned int cbufferOffset, const ShaderValue& value)
		: type(type), rootIndex(rootIndex), tableSize(-1), cbufferOffset(cbufferOffset), defaultValue(value) {}
	ShaderParameter(SHADER_PARAM_TYPE type, unsigned int rootIndex, unsigned int tableSize)
		: type(type), rootIndex(rootIndex), tableSize(tableSize), cbufferOffset(-1), defaultValue(0) {}

	SHADER_PARAM_TYPE Type() const { return type; }
	unsigned int RootIndex() const { return rootIndex; }
	int CBufferOffset() const { return cbufferOffset; }
	int TableSize() const { return tableSize; }
	ShaderValue GetDefaultValue() const { return defaultValue; }

	jstring ToString() const {
		int c = 0;
		char* buf = new char[256];
		switch (type) {
		case SHADER_PARAM_TYPE_CBUFFER:
			c += sprintf_s(buf, 256, "Root CBV");
			break;
		case SHADER_PARAM_TYPE_SRV:
			c += sprintf_s(buf, 256, "Root SRV");
			break;
		case SHADER_PARAM_TYPE_TABLE:
			c += sprintf_s(buf, 256, "Root Table [%d]", tableSize);
			break;

		case SHADER_PARAM_TYPE_FLOATRANGE:
			c += sprintf_s(buf, 256, "Float: %f Range: (%f,%f)", defaultValue.floatValue, defaultValue.floatRange.x, defaultValue.floatRange.y);
			break;
		case SHADER_PARAM_TYPE_INTRANGE:
			c += sprintf_s(buf, 256, "Int: %d Range: (%d,%d)", defaultValue.intValue, defaultValue.intRange.x, defaultValue.intRange.y);
			break;
		case SHADER_PARAM_TYPE_UINTRANGE:
			c += sprintf_s(buf, 256, "UInt: %u Range: (%u,%u)", defaultValue.uintValue, defaultValue.uintRange.x, defaultValue.uintRange.y);
			break;

		case SHADER_PARAM_TYPE_FLOAT:
			c += sprintf_s(buf, 256, "Float:%f", defaultValue.floatValue);
			break;
		case SHADER_PARAM_TYPE_FLOAT2:
			c += sprintf_s(buf, 256, "Float2: %f %f", defaultValue.float2Value.x, defaultValue.float2Value.y);
			break;
		case SHADER_PARAM_TYPE_COLOR3:
			c += sprintf_s(buf, 256, "Color3: %f %f %f", defaultValue.float3Value.x, defaultValue.float3Value.y, defaultValue.float3Value.z);
			break;
		case SHADER_PARAM_TYPE_FLOAT3:
			c += sprintf_s(buf, 256, "Float3: %f %f %f", defaultValue.float3Value.x, defaultValue.float3Value.y, defaultValue.float3Value.z);
			break;
		case SHADER_PARAM_TYPE_COLOR4:
			c += sprintf_s(buf, 256, "Color4: %f %f %f %f", defaultValue.float4Value.x, defaultValue.float4Value.y, defaultValue.float4Value.z, defaultValue.float4Value.w);
			break;
		case SHADER_PARAM_TYPE_FLOAT4:
			c += sprintf_s(buf, 256, "Float4: %f %f %f %f", defaultValue.float4Value.x, defaultValue.float4Value.y, defaultValue.float4Value.z, defaultValue.float4Value.w);
			break;

		case SHADER_PARAM_TYPE_INT:
			c += sprintf_s(buf, 256, "Int: %d", defaultValue.intValue);
			break;
		case SHADER_PARAM_TYPE_INT2:
			c += sprintf_s(buf, 256, "Int2: %d %d", defaultValue.int2Value.x, defaultValue.int2Value.y);
			break;
		case SHADER_PARAM_TYPE_INT3:
			c += sprintf_s(buf, 256, "Int3: %d %d %d", defaultValue.int3Value.x, defaultValue.int3Value.y, defaultValue.int3Value.z);
			break;
		case SHADER_PARAM_TYPE_INT4:
			c += sprintf_s(buf, 256, "Int3: %d %d %d %d", defaultValue.int4Value.x, defaultValue.int4Value.y, defaultValue.int4Value.z, defaultValue.int4Value.w);
			break;

		case SHADER_PARAM_TYPE_UINT:
			c += sprintf_s(buf, 256, "UInt: %u", defaultValue.uintValue);
			break;
		case SHADER_PARAM_TYPE_UINT2:
			c += sprintf_s(buf, 256, "UInt2: %u %u", defaultValue.uint2Value.x, defaultValue.uint2Value.y);
			break;
		case SHADER_PARAM_TYPE_UINT3:
			c += sprintf_s(buf, 256, "UInt3: %u %u %u", defaultValue.uint3Value.x, defaultValue.uint3Value.y, defaultValue.uint3Value.z);
			break;
		case SHADER_PARAM_TYPE_UINT4:
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
		tableSize = rhs.tableSize;
		return *this;
	}

private:
	SHADER_PARAM_TYPE type;
	unsigned int rootIndex;
	int tableSize;
	// for numeric parameters stored in a cbuffer
	int cbufferOffset;
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
	MeshBone(const jwstring& name, DirectX::XMFLOAT4X4 m) : mName(name), mBoneToMesh(m) {}

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

	FontGlyph() : character(L'\0'), advance(0), ox(0), oy(0), tx0(0), ty0(0), tw(0), th(0) {};
	~FontGlyph() {};
};
struct FontKerning {
	wchar_t from;
	wchar_t to;
	int offset;
};

struct ShaderState {
private:
	friend struct std::hash<ShaderState>;

	MESH_SEMANTIC mInput;
	D3D12_RENDER_TARGET_BLEND_DESC mBlendState;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE mTopology;
	D3D12_FILL_MODE mFillMode;
	D3D12_CULL_MODE mCullMode;
	bool mZTest;
	bool mZWrite;
	DXGI_FORMAT mDepthFormat;
	DXGI_FORMAT mRenderFormat;
	unsigned int mMsaaSamples;
	jvector<jstring> mKeywords;

	size_t mHash;

public:
	ShaderState() :
		mInput(MESH_SEMANTIC_POSITION),
		mBlendState(BLEND_STATE_DEFAULT),
		mTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE),
		mZTest(true), mZWrite(true),
		mFillMode(D3D12_FILL_MODE_SOLID),
		mCullMode(D3D12_CULL_MODE_BACK),
		mDepthFormat(DXGI_FORMAT_D32_FLOAT),
		mRenderFormat(DXGI_FORMAT_R8G8B8A8_UNORM),
		mMsaaSamples(1),
		mKeywords(jvector<jstring>()) {
		mHash = (size_t)-1;
	}

	ShaderState(const ShaderState& s) :
		mInput(s.mInput),
		mBlendState(s.mBlendState),
		mTopology(s.mTopology),
		mZTest(s.mZTest), mZWrite(s.mZWrite),
		mFillMode(s.mFillMode),
		mCullMode(s.mCullMode),
		mDepthFormat(s.mDepthFormat),
		mRenderFormat(s.mRenderFormat),
		mMsaaSamples(s.mMsaaSamples),
		mKeywords(s.mKeywords),
		mHash(s.mHash) {}
	~ShaderState() {}

	inline ShaderState& operator =(const ShaderState& rhs) {
		mInput = rhs.mInput;
		mBlendState = rhs.mBlendState;
		mTopology = rhs.mTopology;
		mFillMode = rhs.mFillMode;
		mCullMode = rhs.mCullMode;
		mZTest = rhs.mZTest;
		mZWrite = rhs.mZWrite;
		mDepthFormat = rhs.mDepthFormat;
		mRenderFormat = rhs.mRenderFormat;
		mMsaaSamples = rhs.mMsaaSamples;
		mKeywords = rhs.mKeywords;
		mHash = rhs.mHash;
		return *this;
	}
	inline bool operator ==(const ShaderState& rhs) const {
		if (!(mInput == rhs.mInput &&
			mBlendState == rhs.mBlendState &&
			mTopology == rhs.mTopology &&
			mFillMode == rhs.mFillMode &&
			mCullMode == rhs.mCullMode &&
			mZTest == rhs.mZTest &&
			mZWrite == rhs.mZWrite &&
			mDepthFormat == rhs.mDepthFormat &&
			mRenderFormat == rhs.mRenderFormat &&
			mMsaaSamples == rhs.mMsaaSamples)) return false;

		if (mKeywords.size() != rhs.mKeywords.size()) return false;
		for (unsigned int i = 0; i < rhs.mKeywords.size(); i++)
			if (mKeywords[i] != rhs.mKeywords[i])
				return false;
		return true;
	}

	inline void input(MESH_SEMANTIC x) { mHash = (size_t)-1; mInput = x; }
	inline void blendState(const D3D12_RENDER_TARGET_BLEND_DESC& x) { mHash = (size_t)-1; mBlendState = x; }
	inline void topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE x) { mHash = (size_t)-1; mTopology = x; }
	inline void fillMode(const D3D12_FILL_MODE& x) { mHash = (size_t)-1; mFillMode = x; }
	inline void cullMode(const D3D12_CULL_MODE& x) { mHash = (size_t)-1; mCullMode = x; }
	inline void ztest(bool x) { mHash = (size_t)-1;  mZTest = x; }
	inline void zwrite(bool x) { mHash = (size_t)-1;  mZWrite = x; }
	inline void depthFormat(DXGI_FORMAT x) { mHash = (size_t)-1;  mDepthFormat = x; }
	inline void renderFormat(DXGI_FORMAT x) { mHash = (size_t)-1; mRenderFormat = x; }
	inline void msaaSamples(unsigned int x) { mHash = (size_t)-1; mMsaaSamples = x; }
	inline void addkeyword(const jstring& x) {
		mHash = (size_t)-1;
		mKeywords.push_back(x);
		std::sort(mKeywords.data(), mKeywords.data() + mKeywords.size());
	}
	inline void rmvkeyword(const jstring& x) {
		mHash = (size_t)-1;
		for (unsigned int i = 0; i < mKeywords.size(); i++)
			if (mKeywords[i] == x) {
				mKeywords.remove(i);
				return;
			}
	}
	inline bool haskeyword(const jstring& x) {
		for (unsigned int i = 0; i < mKeywords.size(); i++)
			if (mKeywords[i] == x)
				return true;
		return false;
	}
	inline void setkeywords(const jvector<jstring>& x) {
		mHash = (size_t)-1;
		mKeywords = x;
		std::sort(mKeywords.data(), mKeywords.data() + mKeywords.size());
	}
	inline void clearkeywords() {
		mHash = (size_t)-1;
		mKeywords.clear();
	}

	inline MESH_SEMANTIC input() const { return mInput; }
	inline D3D12_RENDER_TARGET_BLEND_DESC blendState() const { return mBlendState; }
	inline D3D12_PRIMITIVE_TOPOLOGY_TYPE topology() const { return mTopology; }
	inline D3D12_FILL_MODE fillMode() const { return mFillMode; }
	inline D3D12_CULL_MODE cullMode() const { return mCullMode; }
	inline bool ztest() const { return mZTest; }
	inline bool zwrite() const { return mZWrite; }
	inline DXGI_FORMAT depthFormat() const { return mDepthFormat; }
	inline DXGI_FORMAT renderFormat() const { return mRenderFormat; }
	inline unsigned int msaaSamples() const { return mMsaaSamples; }
	inline const jvector<jstring>& keywords() const { return mKeywords; }
};

template <class T>
inline void hash_combine(std::size_t &s, const T &v) {
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

namespace std {
	template<>
	struct hash<ShaderState> {
		std::size_t operator()(const ShaderState &s) const {
			if (s.mHash != (size_t)-1) return s.mHash;
			std::size_t h = 0;
			hash_combine(h, s.mInput);
			hash_combine(h, s.mTopology);
			hash_combine(h, s.mZTest);
			hash_combine(h, s.mZWrite);
			hash_combine(h, s.mFillMode);
			hash_combine(h, s.mCullMode);
			hash_combine(h, s.mBlendState.BlendEnable);
			hash_combine(h, s.mBlendState.BlendOp);
			hash_combine(h, s.mBlendState.BlendOpAlpha);
			hash_combine(h, s.mBlendState.DestBlend);
			hash_combine(h, s.mBlendState.DestBlendAlpha);
			hash_combine(h, s.mBlendState.LogicOp);
			hash_combine(h, s.mBlendState.LogicOpEnable);
			hash_combine(h, s.mBlendState.RenderTargetWriteMask);
			hash_combine(h, s.mBlendState.SrcBlend);
			hash_combine(h, s.mBlendState.SrcBlendAlpha);
			hash_combine(h, s.mDepthFormat);
			hash_combine(h, s.mRenderFormat);
			hash_combine(h, s.mMsaaSamples);
			for (unsigned int i = 0; i < s.mKeywords.size(); i++)
				hash_combine(h, s.mKeywords[i]);
			const_cast<ShaderState*>(&s)->mHash = h;
			return h;
		}
	};
}