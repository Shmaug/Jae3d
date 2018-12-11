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

	JAE_API void Flush(std::shared_ptr<CommandList> commandList);

	JAE_API void DrawLines(jvector<DirectX::XMFLOAT3> vertices, jvector<DirectX::XMFLOAT4> colors);

	JAE_API void DrawText (std::shared_ptr<Font> font, DirectX::XMFLOAT2 pos, float scale, DirectX::XMFLOAT4 color, const wchar_t* text);
	JAE_API void DrawText (std::shared_ptr<Font> font, DirectX::XMFLOAT2 pos, float scale, DirectX::XMFLOAT4 color, jwstring text);
	JAE_API void DrawTextf(std::shared_ptr<Font> font, DirectX::XMFLOAT2 pos, float scale, DirectX::XMFLOAT4 color, jwstring text, ...);

	JAE_API void DrawText (std::shared_ptr<Font> font, DirectX::XMFLOAT4 rect, float scale, DirectX::XMFLOAT4 color, const wchar_t* text);
	JAE_API void DrawText (std::shared_ptr<Font> font, DirectX::XMFLOAT4 rect, float scale, DirectX::XMFLOAT4 color, jwstring text);
	JAE_API void DrawTextf(std::shared_ptr<Font> font, DirectX::XMFLOAT4 rect, float scale, DirectX::XMFLOAT4 color, jwstring text, ...);

private:
	class SpriteDraw {
	public:
		std::shared_ptr<Texture> mTexture;
		DirectX::XMFLOAT4 mPixelRect;
		DirectX::XMFLOAT4 mTextureRect;
		DirectX::XMFLOAT4 mColor;

		SpriteDraw() : mTexture(nullptr), mPixelRect({ 0,0,0,0 }), mTextureRect({ 0,0,0,0 }), mColor({ 1,1,1,1 }) {}
		SpriteDraw(std::shared_ptr<Texture> tex, DirectX::XMFLOAT4 rect, DirectX::XMFLOAT4 tRect, DirectX::XMFLOAT4 color) : mTexture(tex), mPixelRect(rect), mTextureRect(tRect), mColor(color) {}
		SpriteDraw(const SpriteDraw &t) : mTexture(t.mTexture), mPixelRect(t.mPixelRect), mTextureRect(t.mTextureRect), mColor(t.mColor) {}
		~SpriteDraw() {}

		SpriteDraw& operator=(const SpriteDraw &rhs) {
			mTexture = rhs.mTexture;
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
		uint16_t*     mMappedQuadIndices;
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

		SpriteContext() {};
		~SpriteContext() { Release(); }

		JAE_API void ResizeQuads(size_t length);
		JAE_API void ResizeLines(size_t length);
		JAE_API void AddQuad(DirectX::XMFLOAT4 screenRect, DirectX::XMFLOAT4 texRect, DirectX::XMFLOAT4 color);
		JAE_API void AddLines(LineDraw &line);
		JAE_API void Reset();
		JAE_API void Release();

		JAE_API void DrawQuadGroup(_WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<Texture> tex, unsigned int startQuad);
		JAE_API void DrawLines(_WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, unsigned int startIndex);
	};

	jvector<SpriteDraw> mQuadDrawQueue;
	jvector<LineDraw> mLineDrawQueue;

	JAE_API void Release();
	JAE_API void CreateShader();
	JAE_API SpriteContext* GetContext(unsigned int frameIndex);

	// resources
	std::shared_ptr<Shader> mColoredShader;
	std::shared_ptr<Shader> mTexturedShader;
	jvector<SpriteContext*> mContexts;
};