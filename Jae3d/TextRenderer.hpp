#pragma once

#include "Util.hpp"

#include <wrl.h>

#include <d3d12.h>
#include <memory>

#include "Object.hpp"
#include "Renderer.hpp"

class ConstantBuffer;
class Font;
class Material;

class TextRenderer : public Renderer {
private:
	struct TextVertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 binormal;
		DirectX::XMFLOAT4 tex;

		TextVertex() : position(DirectX::XMFLOAT3()), normal(DirectX::XMFLOAT3()), tangent(DirectX::XMFLOAT3()), binormal(DirectX::XMFLOAT3()), tex(DirectX::XMFLOAT4()) {}
		TextVertex(float x, float y, float u, float v)
			: position({ x, y, 0 }), normal({ 0, 0, -1 }), tangent({ 1, 0, 0 }), binormal({ 0, 1, 0 }), tex({ u, v, 0, 0 }) {};
	};
	struct TextMesh {
		unsigned int mMappedLength; // in quads
		unsigned int mQuadCount;

		_WRL::ComPtr<ID3D12Resource> mVertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
		TextVertex* mMappedVertices;

		_WRL::ComPtr<ID3D12Resource> mIndexBuffer;
		D3D12_INDEX_BUFFER_VIEW  mIndexBufferView;
		uint16_t*   mMappedIndices;

		std::shared_ptr<Material> mMaterial;
		std::shared_ptr<Font> mFont;
		jwstring mText;

		TextMesh() : mMappedLength(0), mQuadCount(0), mText(L""),
			mVertexBuffer(nullptr), mVertexBufferView({}), mMappedVertices(nullptr),
			mIndexBuffer(nullptr), mIndexBufferView({}), mMappedIndices(nullptr),
			mFont(nullptr), mMaterial(nullptr) {}
	};

public:
	class TextRenderJob : public RenderJob {
	public:
		unsigned int mSubmesh;
		const TextMesh& mMesh;
		std::shared_ptr<::Material> mMaterial;
		std::shared_ptr<ConstantBuffer> mObjectBuffer;

		JAE_API TextRenderJob(unsigned int queue, const jwstring& batch, const TextMesh& mesh, const std::shared_ptr<::Material>& mat, const std::shared_ptr<ConstantBuffer>& buf);

		JAE_API RenderJob* Batch(RenderJob* other, const std::shared_ptr<CommandList>& commandList) override;
		JAE_API void Execute(const std::shared_ptr<CommandList>& commandList, const std::shared_ptr<::Material>& materialOverride) override;
	};

	JAE_API TextRenderer();
	JAE_API TextRenderer(const jwstring& name);
	JAE_API ~TextRenderer();

	inline std::shared_ptr<::Font> Font() const { return mFont; }
	inline void Font(const std::shared_ptr<::Font>& font) { mFont = font; }

	inline jwstring Text() const { return mText; }
	inline void Text(const jwstring& text) { mText = text; }

	inline std::shared_ptr<Material> Material() { return mMaterial; };
	inline void Material(const std::shared_ptr<::Material>& material) { mMaterial = material; }

	JAE_API void GatherRenderJobs(const std::shared_ptr<CommandList>& commandList, const std::shared_ptr<Camera>& camera, jvector<RenderJob*>& list) override;
	bool Visible() override { return mVisible; }
	JAE_API DirectX::BoundingOrientedBox Bounds() override;

	bool mVisible;
	jwstring mBatchGroup;

private:
	std::shared_ptr<ConstantBuffer> mCBuffer;
	std::shared_ptr<::Font> mFont;
	std::shared_ptr<::Material> mMaterial;
	TextMesh* mMesh;
	float mTextScale;

	DirectX::BoundingBox mBounds;
	jwstring mText;

	JAE_API void Build(TextMesh& mesh);
	JAE_API void AddQuad(jvector<TextVertex>& vertices, jvector<uint16_t>& indices, DirectX::XMFLOAT4 rect, DirectX::XMFLOAT4 uvRect);
};