#pragma once

#include "Common.hpp"
#include <DirectXCollision.h>
#include "Asset.hpp"

class Mesh : public Asset {
public:
	JAE_API Mesh(jwstring name);
	JAE_API Mesh(jwstring name, MemoryStream &ms);
	JAE_API ~Mesh();

	JAE_API void LoadCube(float size);
	JAE_API void LoadQuad(float size);
	// Copies the mesh to the GPU
	JAE_API void UploadStatic();
	JAE_API void ReleaseGpu();

	JAE_API void WriteData(MemoryStream &ms);
	JAE_API uint64_t TypeId();

#pragma region Getters/Setters
	JAE_API void Clear();

	JAE_API void Use32BitIndices(bool v);
	bool Use32BitIndices() const { return m32BitIndices; }
	void AddTriangle(uint32_t i0, uint32_t i1, uint32_t i2) {
		if (m32BitIndices) {
			mIndices32.push_back(i0);
			mIndices32.push_back(i1);
			mIndices32.push_back(i2);
		} else {
			mIndices16.push_back((uint16_t)i0);
			mIndices16.push_back((uint16_t)i1);
			mIndices16.push_back((uint16_t)i2);
		}
	}

	unsigned int VertexCount() const { return (unsigned int)mVertices.size(); }
	unsigned int IndexCount() const { return(unsigned int)(m32BitIndices ? mIndices32.size() : mIndices16.size()); }
	JAE_API void VertexCount(unsigned int size, bool shrink = true);

	JAE_API void AddIndices(unsigned int count, unsigned int* indices);
	JAE_API unsigned int AddVertex(DirectX::XMFLOAT3 &v);

	JAE_API void HasSemantic(MESH_SEMANTIC s, bool v);
	bool HasSemantic(MESH_SEMANTIC s) const { if (s == MESH_SEMANTIC_POSITION) return true; return mSemantics & s; }
	MESH_SEMANTIC Semantics() const { return mSemantics; }

	template<typename T>
	T* GetSemantic(MESH_SEMANTIC s) {
		switch (s) {
		case MESH_SEMANTIC_POSITION:
			static_assert(std::is_same(T, DirectX::XMFLOAT3), "T must be XMFLOAT3");
			return mVertices.data();
			break;
		case MESH_SEMANTIC_NORMAL:
			static_assert(std::is_same(T, DirectX::XMFLOAT3), "T must be XMFLOAT3");
			return mNormals.data();
			break;
		case MESH_SEMANTIC_TANGENT:
			static_assert(std::is_same(T, DirectX::XMFLOAT3), "T must be XMFLOAT3");
			return mTangents.data();
			break;
		case SEMANTIC_BINORMAL:
			static_assert(std::is_same(T, DirectX::XMFLOAT3), "T must be XMFLOAT3");
			return mBinormals.data();
			break;
		case SEMANTIC_COLOR0:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return mColor0.data();
			break;
		case SEMANTIC_COLOR1:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return mColor1.data();
			break;
		case SEMANTIC_BLENDINDICES:
			static_assert(std::is_same(T, DirectX::XMUINT4), "T must be XMUINT4");
			return mBlendIndices.data();
			break;
		case SEMANTIC_BLENDWEIGHT:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return mBlendWeights.data();
			break;
		case SEMANTIC_TEXCOORD0:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return mTexcoord0.data();
			break;
		case SEMANTIC_TEXCOORD1:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return mTexcoord1.data();
			break;
		case SEMANTIC_TEXCOORD2:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return mTexcoord2.data();
			break;
		case SEMANTIC_TEXCOORD3:
			static_assert(std::is_same(T, DirectX::XMFLOAT4), "T must be XMFLOAT4");
			return mTexcoord3.data();
			break;
		}
	}

	inline DirectX::BoundingBox Bounds() const { return mBounds; }

	inline DirectX::XMFLOAT3* GetVertices() const { return mVertices.data(); }
	inline DirectX::XMFLOAT3* GetNormals() const { return mNormals.data(); }
	inline DirectX::XMFLOAT3* GetTangents() const { return mTangents.data(); }
	inline DirectX::XMFLOAT3* GetBinormals() const { return mBinormals.data(); }
	inline DirectX::XMFLOAT4* GetTexcoords(const int channel) const {
		switch (channel) {
		default:
		case 0: return mTexcoord0.data();
		case 1: return mTexcoord1.data();
		case 2: return mTexcoord2.data();
		case 3: return mTexcoord3.data();
		}
	}
	inline DirectX::XMFLOAT4* GetColors(const int channel) const {
		switch (channel) {
		default:
		case 0: return mColor0.data();
		case 1: return mColor1.data();
		}
	}
	inline DirectX::XMUINT4*  GetBlendIndices() const { return mBlendIndices.data(); }
	inline DirectX::XMFLOAT4* GetBlendWeights() const { return mBlendWeights.data(); }

	uint16_t* GetIndices16() const { return mIndices16.data(); }
	uint32_t* GetIndices32() const { return mIndices32.data(); }
#pragma endregion

	JAE_API void Draw(_WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

private:
	bool m32BitIndices = false;
	bool mDataUploaded = false;
	bool mDataMapped = false;

	DirectX::BoundingBox mBounds;

	MESH_SEMANTIC mSemantics = MESH_SEMANTIC_POSITION;

	jvector<MeshBone> bones;
	jvector<DirectX::XMFLOAT3> mVertices;
	jvector<DirectX::XMFLOAT3> mNormals;
	jvector<DirectX::XMFLOAT3> mTangents;
	jvector<DirectX::XMFLOAT3> mBinormals;
	jvector<DirectX::XMFLOAT4> mTexcoord0;
	jvector<DirectX::XMFLOAT4> mTexcoord1;
	jvector<DirectX::XMFLOAT4> mTexcoord2;
	jvector<DirectX::XMFLOAT4> mTexcoord3;
	jvector<DirectX::XMFLOAT4> mColor0;
	jvector<DirectX::XMFLOAT4> mColor1;
	jvector<DirectX::XMUINT4>  mBlendIndices;
	jvector<DirectX::XMFLOAT4> mBlendWeights;
	jvector<uint16_t> mIndices16;
	jvector<uint32_t> mIndices32;

	JAE_API size_t VertexSize();
	JAE_API void WriteVertexArray(BYTE* dst);

	_WRL::ComPtr<ID3D12Resource> mVertexBuffer;
	_WRL::ComPtr<ID3D12Resource> mIndexBuffer;
	void* mMappedVertexBuffer;
	void* mMappedIndexBuffer;
	UINT mIndexCount;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
};