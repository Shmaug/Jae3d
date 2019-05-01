#pragma once

#include "Common.hpp"

#include "Texture.hpp"
#include "Font.hpp"

#ifdef DrawText
#undef DrawText
#endif

class CommandList;

class SpriteBatch {
public:
	~SpriteBatch();

	JAE_API void Reset(std::shared_ptr<CommandList> commandList);
	JAE_API void Flush(std::shared_ptr<CommandList> commandList, std::shared_ptr<Shader> shaderOverride = nullptr);

	JAE_API void DrawLines(const jvector<DirectX::XMFLOAT3>& vertices, const jvector<DirectX::XMFLOAT4>& colors);

	JAE_API DirectX::XMFLOAT2 MeasureText(std::shared_ptr<Font> font, const float &scale, const jwstring& text);

	JAE_API void DrawText (std::shared_ptr<Font> font, const DirectX::XMFLOAT2& pos, const float &scale, const DirectX::XMFLOAT4& color, const jwstring& text);
	JAE_API void DrawTextf(std::shared_ptr<Font> font, const DirectX::XMFLOAT2& pos, const float &scale, const DirectX::XMFLOAT4& color, jwstring text, ...);

	JAE_API void SpriteBatch::DrawTexture(_WRL::ComPtr<ID3D12DescriptorHeap> srvHeap, D3D12_GPU_DESCRIPTOR_HANDLE srv, const DirectX::XMFLOAT4& rect, const DirectX::XMFLOAT4& color = { 1, 1, 1, 1 }, const DirectX::XMFLOAT4& srcRect = { 0, 0, 1, 1 });
	JAE_API void SpriteBatch::DrawTexture(std::shared_ptr<Texture> texture, const DirectX::XMFLOAT4& rect, const DirectX::XMFLOAT4& color = { 1, 1, 1, 1 }, const DirectX::XMFLOAT4& srcRect = { 0, 0, 1, 1 });

private:
	class SpriteDraw {
	public:
		_WRL::ComPtr<ID3D12DescriptorHeap> mTextureHeap;
		D3D12_GPU_DESCRIPTOR_HANDLE mTextureSRV;
		DirectX::XMFLOAT4 mPixelRect;
		DirectX::XMFLOAT4 mTextureRect;
		DirectX::XMFLOAT4 mColor;

		SpriteDraw() : mTextureHeap(nullptr), mPixelRect({ 0,0,0,0 }), mTextureRect({ 0,0,0,0 }), mColor({ 1,1,1,1 }), mTextureSRV({}) {}
		SpriteDraw(_WRL::ComPtr<ID3D12DescriptorHeap> srvHeap, D3D12_GPU_DESCRIPTOR_HANDLE srv, DirectX::XMFLOAT4 rect, DirectX::XMFLOAT4 tRect, DirectX::XMFLOAT4 color)
			: mTextureHeap(srvHeap), mTextureSRV(srv), mPixelRect(rect), mTextureRect(tRect), mColor(color) {}
		SpriteDraw(const SpriteDraw &t)
			: mTextureHeap(t.mTextureHeap), mTextureSRV(t.mTextureSRV), mPixelRect(t.mPixelRect), mTextureRect(t.mTextureRect), mColor(t.mColor) {}
		~SpriteDraw() {}

		SpriteDraw& operator=(const SpriteDraw &rhs) {
			mTextureHeap = rhs.mTextureHeap;
			mTextureSRV = rhs.mTextureSRV;
			mPixelRect = rhs.mPixelRect;
			mTextureRect = rhs.mTextureRect;
			mColor = rhs.mColor;
		}
	};
	class LineDraw {
	public:
		jvector<DirectX::XMFLOAT3> mVertices;
		jvector<DirectX::XMFLOAT4> mColors;
	};
	
	class SpriteContext {
	public:
		struct QuadVertex {
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT2 tex;
		};
		struct LineVertex {
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT4 color;
		};

		// Quad dynamic mesh
		_WRL::ComPtr<ID3D12Resource> mQuadVertexBuffer;
		_WRL::ComPtr<ID3D12Resource> mQuadIndexBuffer;
		D3D12_VERTEX_BUFFER_VIEW mQuadVertexBufferView;
		D3D12_INDEX_BUFFER_VIEW  mQuadIndexBufferView;
		QuadVertex* mMappedQuadVertices;
		uint16_t*   mMappedQuadIndices;
		size_t mMappedQuadLength;
		unsigned int mQuadOffset;

		// Line dynamic mesh
		_WRL::ComPtr<ID3D12Resource> mLineVertexBuffer;
		_WRL::ComPtr<ID3D12Resource> mLineIndexBuffer;
		D3D12_VERTEX_BUFFER_VIEW mLineVertexBufferView;
		D3D12_INDEX_BUFFER_VIEW  mLineIndexBufferView;
		LineVertex* mMappedLineVertices;
		uint16_t*   mMappedLineIndices;
		size_t mMappedLineLength;
		unsigned int mLineVertexOffset;
		unsigned int mLineIndexOffset;

		unsigned int mFrameIndex;
		bool mActive;

		SpriteContext()
			: mActive(false), mMappedQuadVertices(nullptr), mMappedQuadIndices(nullptr), mMappedQuadLength(0), mQuadOffset(0),
			mMappedLineVertices(nullptr), mMappedLineIndices(nullptr), mMappedLineLength(0), mLineVertexOffset(0), mLineIndexOffset(0),
			mQuadVertexBufferView({}), mQuadIndexBufferView({}), mLineIndexBufferView({}), mLineVertexBufferView({}), mFrameIndex(0) {};
		~SpriteContext() { Release(); }

		JAE_API void ResizeQuads(unsigned int length);
		JAE_API void ResizeLines(unsigned int length);
		JAE_API void AddQuad(const DirectX::XMFLOAT4& screenRect, const DirectX::XMFLOAT4& texRect, const DirectX::XMFLOAT4& color);
		JAE_API void AddLines(const LineDraw &line);
		JAE_API void Reset();
		JAE_API void Release();

		JAE_API void SpriteBatch::SpriteContext::DrawQuadGroup(std::shared_ptr<CommandList> cmdList, unsigned int startQuad, _WRL::ComPtr<ID3D12DescriptorHeap> heap, D3D12_GPU_DESCRIPTOR_HANDLE tex, const DirectX::XMFLOAT4X4& matrix);
		JAE_API void DrawLines(_WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, unsigned int startIndex, const DirectX::XMFLOAT4X4& mat);
	};

	unsigned int mQuadDrawQueueCount;
	unsigned int mLineDrawQueueCount;
	jvector<SpriteDraw> mQuadDrawQueue;
	jvector<LineDraw> mLineDrawQueue;

	JAE_API void Release();
	JAE_API void CreateShader();
	JAE_API SpriteContext* GetContext(unsigned int frameIndex);

	// resources
	std::shared_ptr<Shader> mColoredShader;
	std::shared_ptr<Shader> mTexturedShader;
	jvector<jvector<SpriteContext*>> mContexts;
};