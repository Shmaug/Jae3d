#include "SpriteBatch.hpp"

#include <wrl.h>
#include "Graphics.hpp"
#include "CommandList.hpp"
#include "AssetDatabase.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#include "Profiler.hpp"

using namespace Microsoft::WRL;
using namespace DirectX;

SpriteBatch::~SpriteBatch() { Release(); }

void SpriteBatch::Release() {
	for (unsigned int i = 0; i < mContexts.size(); i++)
		for (unsigned int j = 0; j < mContexts[i].size(); j++)
			delete mContexts[i][j];
	mContexts.free();
}
void SpriteBatch::SpriteContext::Release() {
	if (mMappedQuadLength) {
		CD3DX12_RANGE readRange(0, 0);
		mQuadVertexBuffer->Unmap(0, &readRange);
		mQuadIndexBuffer->Unmap(0, &readRange);
		mMappedQuadLength = 0;
	}
	mQuadVertexBuffer.Reset();
	mQuadIndexBuffer.Reset();

	if (mMappedLineLength) {
		CD3DX12_RANGE readRange(0, 0);
		mLineVertexBuffer->Unmap(0, &readRange);
		mLineIndexBuffer->Unmap(0, &readRange);
		mMappedLineLength = 0;
	}
	mLineVertexBuffer.Reset();
	mLineIndexBuffer.Reset();
}
void SpriteBatch::SpriteContext::ResizeQuads(unsigned int length) {
	if (mMappedQuadLength >= length) return;
	auto device = Graphics::GetDevice();

	UINT vsize = (UINT)length * sizeof(QuadVertex) * 4;
	UINT isize = (UINT)length * sizeof(uint16_t) * 6;

	// Vertices
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vsize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&mQuadVertexBuffer)));

	mQuadVertexBuffer->SetName(L"Quad Vertex Buffer");

	void* vdata;
	CD3DX12_RANGE readRange(0, 0);
	mQuadVertexBuffer->Map(0, &readRange, &vdata);

	mQuadVertexBufferView.BufferLocation = mQuadVertexBuffer->GetGPUVirtualAddress();
	mQuadVertexBufferView.SizeInBytes = vsize;
	mQuadVertexBufferView.StrideInBytes = sizeof(QuadVertex);

	mMappedQuadVertices = reinterpret_cast<QuadVertex*>(vdata);

	// Indices
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(isize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&mQuadIndexBuffer)));

	mQuadIndexBuffer->SetName(L"Quad Index Buffer");

	void* idata;
	mQuadIndexBuffer->Map(0, &readRange, &idata);

	mQuadIndexBufferView.BufferLocation = mQuadIndexBuffer->GetGPUVirtualAddress();
	mQuadIndexBufferView.SizeInBytes = isize;
	mQuadIndexBufferView.Format = DXGI_FORMAT_R16_UINT;

	mMappedQuadIndices = reinterpret_cast<uint16_t*>(idata);

	mMappedQuadLength = length;
}
void SpriteBatch::SpriteContext::ResizeLines(unsigned int length) {
	if (mMappedLineLength >= length) return;
	auto device = Graphics::GetDevice();

	UINT vsize = (UINT)length * sizeof(LineVertex);
	UINT isize = (UINT)length * sizeof(uint16_t);

	// Vertices
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vsize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&mLineVertexBuffer)));

	mLineVertexBuffer->SetName(L"Line Vertex Buffer");

	void* vdata;
	CD3DX12_RANGE readRange(0, 0);
	mLineVertexBuffer->Map(0, &readRange, &vdata);

	mLineVertexBufferView.BufferLocation = mLineVertexBuffer->GetGPUVirtualAddress();
	mLineVertexBufferView.SizeInBytes = vsize;
	mLineVertexBufferView.StrideInBytes = sizeof(LineVertex);

	mMappedLineVertices = reinterpret_cast<LineVertex*>(vdata);

	// Indices
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(isize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&mLineIndexBuffer)));

	mLineIndexBuffer->SetName(L"Line Index Buffer");

	void* idata;
	mLineIndexBuffer->Map(0, &readRange, &idata);

	mLineIndexBufferView.BufferLocation = mLineIndexBuffer->GetGPUVirtualAddress();
	mLineIndexBufferView.SizeInBytes = isize;
	mLineIndexBufferView.Format = DXGI_FORMAT_R16_UINT;

	mMappedLineIndices = reinterpret_cast<uint16_t*>(idata);

	mMappedLineLength = length;
}
void SpriteBatch::SpriteContext::Reset() {
	mQuadOffset = 0;
	mLineVertexOffset = 0;
	mLineIndexOffset = 0;
	mActive = false;
}

void SpriteBatch::DrawLines(const jvector<XMFLOAT3> &vertices, const jvector<XMFLOAT4> &colors) {
	if (mLineDrawQueueCount >= mLineDrawQueue.size()) mLineDrawQueue.push_back({});
	LineDraw &s = mLineDrawQueue[mLineDrawQueueCount++];
	s.mColors = colors;
	s.mVertices = vertices;
}

XMFLOAT2 SpriteBatch::MeasureText(std::shared_ptr<Font> font, const float& scale, const jwstring& text) {
	float w = (float)font->GetTexture()->Width();
	float h = (float)font->GetTexture()->Height();
	float sc = scale * (float)Graphics::GetWindow()->GetLogPixelsX() / font->GetTextureDpi();
	XMFLOAT2 m(0, 0);
	XMFLOAT2 p(0, 0);
	FontGlyph g;
	wchar_t prev = L'\0';
	for (unsigned int j = 0; j < text.length(); j++) {
		wchar_t cur = text[j];
		if (cur == L'\n') {
			p.x = 0;
			p.y += sc * font->GetLineSpacing();
		} else {
			if (!font->GetGlyph(cur, g) && !font->GetGlyph(L'?', g)) {
				prev = cur;
				continue;
			}

			p.x += sc * font->GetKerning(prev, cur);
			p.x += sc * g.advance;
		}
		m.x = fmax(m.x, p.x);
		m.y = fmax(m.y, p.y);
		prev = cur;
	}
	return m;
}

void SpriteBatch::DrawText (std::shared_ptr<Font> font, const XMFLOAT2& pos, const float &scale, const XMFLOAT4& color, const jwstring& text) {
	float w = (float)font->GetTexture()->Width();
	float h = (float)font->GetTexture()->Height();
	float sc = scale * (float)Graphics::GetWindow()->GetLogPixelsX() / font->GetTextureDpi();
	XMFLOAT2 p = pos;
	wchar_t prev = L'\0';
	FontGlyph g;
	for (unsigned int j = 0; j < text.length(); j++) {
		wchar_t cur = text[j];
		if (cur == L'\n') {
			p.x = pos.x;
			p.y += sc * font->GetLineSpacing();
		} else {
			Profiler::BeginSample(L"Find Glyph", true);
			if (!font->GetGlyph(cur, g)) {
				prev = cur;
				Profiler::EndSample();
				continue;
			}
			Profiler::EndSample();

			Profiler::BeginSample(L"Kerning", true);
			p.x += sc * font->GetKerning(prev, cur);
			Profiler::EndSample();

			if (g.character != L' ') {
				Profiler::BeginSample(L"Submit SpriteDraw", true);

				if (mQuadDrawQueueCount >= mQuadDrawQueue.size()) mQuadDrawQueue.push_back({});
				SpriteDraw &s = mQuadDrawQueue[mQuadDrawQueueCount++];
				s.mColor = color;
				s.mPixelRect = XMFLOAT4(
					p.x + sc * g.ox,
					p.y - sc * g.oy,
					p.x + sc * (g.tw + g.ox),
					p.y + sc * (g.th - g.oy));
				s.mTextureRect = XMFLOAT4((float)g.tx0 / w, (float)g.ty0 / h, (float)(g.tx0 + g.tw) / w, (float)(g.ty0 + g.th) / h);
				s.mTextureSRV = font->GetTexture()->GetSRVGPUDescriptor();
				s.mTextureHeap = font->GetTexture()->GetSRVDescriptorHeap();

				Profiler::EndSample();
			}

			p.x += sc * g.advance;
		}
		prev = cur;
	}
}
void SpriteBatch::DrawTextf(std::shared_ptr<Font> font, const XMFLOAT2& pos, const float &scale, const XMFLOAT4& color, jwstring fmt, ...) {
	wchar_t buf[1025];
	va_list args;
	va_start(args, fmt);
	wvsprintf(buf, fmt.c_str(), args);
	va_end(args);
	DrawText(font, pos, scale, color, jwstring(buf));
}

void SpriteBatch::DrawTexture(ComPtr<ID3D12DescriptorHeap> mTextureHeap, D3D12_GPU_DESCRIPTOR_HANDLE mTextureSRV, const XMFLOAT4& rect, const XMFLOAT4& color, const XMFLOAT4& srcRect) {
	if (mQuadDrawQueueCount >= mQuadDrawQueue.size()) mQuadDrawQueue.push_back({});
	SpriteDraw &s = mQuadDrawQueue[mQuadDrawQueueCount++];
	s.mColor = color;
	s.mPixelRect = rect;
	s.mTextureRect = srcRect;
	s.mTextureSRV = mTextureSRV;
	s.mTextureHeap = mTextureHeap;
}
void SpriteBatch::DrawTexture(std::shared_ptr<Texture> texture, const XMFLOAT4& rect, const XMFLOAT4& color, const XMFLOAT4& srcRect) {
	if (mQuadDrawQueueCount >= mQuadDrawQueue.size()) mQuadDrawQueue.push_back({});
	SpriteDraw &s = mQuadDrawQueue[mQuadDrawQueueCount++];
	s.mColor = color;
	s.mPixelRect = rect;
	s.mTextureRect = srcRect;
	s.mTextureSRV = texture->GetSRVGPUDescriptor();
	s.mTextureHeap = texture->GetSRVDescriptorHeap();
}

SpriteBatch::SpriteContext* SpriteBatch::GetContext(unsigned int frameIndex) {
	if (frameIndex < mContexts.size()) {
		unsigned int i = 0;
		SpriteContext* ctx = nullptr;
		do {
			ctx = mContexts[frameIndex][i];
			i++;
		} while (ctx->mActive && i < mContexts[frameIndex].size());
		if (i >= mContexts[frameIndex].size())
			ctx = mContexts[frameIndex].push_back(new SpriteContext());
		ctx->Reset();
		return ctx;
	}
	// create contexts for each frame
	while (mContexts.size() < frameIndex + 1)
		mContexts.push_back(jvector<SpriteContext*>()).push_back(new SpriteContext());
	return mContexts[frameIndex][0];
}

void SpriteBatch::SpriteContext::AddQuad(const XMFLOAT4& screenRect, const XMFLOAT4& texRect, const XMFLOAT4& color) {
	unsigned int v = (unsigned int)mQuadOffset * 4;
	unsigned int i = (unsigned int)mQuadOffset * 6;

	mMappedQuadIndices[i++] = v + 0;
	mMappedQuadIndices[i++] = v + 1;
	mMappedQuadIndices[i++] = v + 2;
	mMappedQuadIndices[i++] = v + 1;
	mMappedQuadIndices[i++] = v + 3;
	mMappedQuadIndices[i++] = v + 2;

	mMappedQuadVertices[v].position.x = screenRect.x;
	mMappedQuadVertices[v].position.y = screenRect.y;
	mMappedQuadVertices[v].position.z = 0;
	mMappedQuadVertices[v].color.x    = color.x;
	mMappedQuadVertices[v].color.y    = color.y;
	mMappedQuadVertices[v].color.z    = color.z;
	mMappedQuadVertices[v].color.w    = color.w;
	mMappedQuadVertices[v].tex.x      = texRect.x;
	mMappedQuadVertices[v].tex.y      = texRect.y;
	v++;

	mMappedQuadVertices[v].position.x = screenRect.z;
	mMappedQuadVertices[v].position.y = screenRect.y;
	mMappedQuadVertices[v].position.z = 0;
	mMappedQuadVertices[v].color.x = color.x;
	mMappedQuadVertices[v].color.y = color.y;
	mMappedQuadVertices[v].color.z = color.z;
	mMappedQuadVertices[v].color.w = color.w;
	mMappedQuadVertices[v].tex.x = texRect.z;
	mMappedQuadVertices[v].tex.y = texRect.y;
	v++;

	mMappedQuadVertices[v].position.x = screenRect.x;
	mMappedQuadVertices[v].position.y = screenRect.w;
	mMappedQuadVertices[v].position.z = 0;
	mMappedQuadVertices[v].color.x = color.x;
	mMappedQuadVertices[v].color.y = color.y;
	mMappedQuadVertices[v].color.z = color.z;
	mMappedQuadVertices[v].color.w = color.w;
	mMappedQuadVertices[v].tex.x = texRect.x;
	mMappedQuadVertices[v].tex.y = texRect.w;
	v++;

	mMappedQuadVertices[v].position.x = screenRect.z;
	mMappedQuadVertices[v].position.y = screenRect.w;
	mMappedQuadVertices[v].position.z = 0;
	mMappedQuadVertices[v].color.x = color.x;
	mMappedQuadVertices[v].color.y = color.y;
	mMappedQuadVertices[v].color.z = color.z;
	mMappedQuadVertices[v].color.w = color.w;
	mMappedQuadVertices[v].tex.x = texRect.z;
	mMappedQuadVertices[v].tex.y = texRect.w;
	v++;

	mQuadOffset++;
}
void SpriteBatch::SpriteContext::AddLines(const LineDraw &lines) {
	for (int i = 0; i < lines.mVertices.size(); i++) {
		if (i > 0) {
			mMappedLineIndices[mLineIndexOffset++] = mLineVertexOffset - 1;
			mMappedLineIndices[mLineIndexOffset++] = mLineVertexOffset;
		}

		mMappedLineVertices[mLineVertexOffset].position = lines.mVertices[i];
		mMappedLineVertices[mLineVertexOffset++].color = lines.mColors[i];
	}
}

void SpriteBatch::CreateShader() {
	mTexturedShader = AssetDatabase::GetAsset<Shader>(L"SpriteTextured");
	mTexturedShader->Upload();
	mColoredShader = AssetDatabase::GetAsset<Shader>(L"SpriteColored");
	mColoredShader->Upload();
}

void SpriteBatch::SpriteContext::DrawQuadGroup(std::shared_ptr<CommandList> commandList, unsigned int startQuad, ComPtr<ID3D12DescriptorHeap> heap, D3D12_GPU_DESCRIPTOR_HANDLE tex, const XMFLOAT4X4& mat) {
	auto cmdList = commandList->D3DCommandList();

	XMFLOAT4 c{ 1, 1, 1, 1 };
	cmdList->SetGraphicsRoot32BitConstants(0, 16, &mat, 0);
	cmdList->SetGraphicsRoot32BitConstants(0, 4, &c, 16);

	ID3D12DescriptorHeap* heaps = { heap.Get() };
	cmdList->SetDescriptorHeaps(1, &heaps);
	cmdList->SetGraphicsRootDescriptorTable(1, tex);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &mQuadVertexBufferView);
	cmdList->IASetIndexBuffer(&mQuadIndexBufferView);
	cmdList->DrawIndexedInstanced((mQuadOffset - startQuad) * 6, 1, startQuad * 6, 0, 0);
}
void SpriteBatch::SpriteContext::DrawLines(ComPtr<ID3D12GraphicsCommandList> cmdList, unsigned int startIndex, const XMFLOAT4X4 &mat) {
	XMFLOAT4 c{ 1, 1, 1, 1 };
	cmdList->SetGraphicsRoot32BitConstants(0, 16, &mat, 0);
	cmdList->SetGraphicsRoot32BitConstants(0, 4, &c, 16);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	cmdList->IASetVertexBuffers(0, 1, &mLineVertexBufferView);
	cmdList->IASetIndexBuffer(&mLineIndexBufferView);
	cmdList->DrawIndexedInstanced(mLineIndexOffset - startIndex, 1, startIndex, 0, 0);
}

void SpriteBatch::Reset(std::shared_ptr<CommandList> commandList) {
	if (commandList->GetFrameIndex() < mContexts.size())
		for (unsigned int j = 0; j < mContexts[commandList->GetFrameIndex()].size(); j++)
			mContexts[commandList->GetFrameIndex()][j]->Reset();
}

void SpriteBatch::Flush(std::shared_ptr<CommandList> commandList, std::shared_ptr<Shader> shaderOverride) {
	SpriteContext* ctx = GetContext(commandList->GetFrameIndex());

	XMFLOAT4X4 screenMVP;
	XMStoreFloat4x4(&screenMVP, XMMatrixOrthographicOffCenterLH(0, (float)commandList->CurrentRenderTargetWidth(), (float)commandList->CurrentRenderTargetHeight(), 0, 0, 1.0f));

	// Draw quads
	if (mQuadDrawQueueCount > 0) {
		ctx->ResizeQuads(mQuadDrawQueueCount);

		unsigned int curq = 0;
		ComPtr<ID3D12DescriptorHeap> curHeap = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE curSRV = { 0 };

		commandList->SetMaterial(nullptr);
		if (!shaderOverride) {
			if (!mTexturedShader) CreateShader();
			commandList->SetShader(mTexturedShader);
		} else {
			commandList->SetShader(shaderOverride);
		}
		commandList->SetBlendState(BLEND_STATE_ALPHA);
		commandList->SetCullMode(D3D12_CULL_MODE_NONE);
		commandList->DrawUserMesh((MESH_SEMANTIC)(MESH_SEMANTIC_POSITION | MESH_SEMANTIC_TEXCOORD0 | MESH_SEMANTIC_COLOR0), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		// bundle together consecutive draws with the same texture (heap/srv) into a single draw call
		for (unsigned int i = 0; i < mQuadDrawQueueCount; i++) {
			const SpriteDraw& s = mQuadDrawQueue[i];

			if (curSRV.ptr != s.mTextureSRV.ptr) {
				// Draw previous batch
				if (curHeap && curSRV.ptr)
					ctx->DrawQuadGroup(commandList, curq, curHeap, curSRV, screenMVP);
				// Start new batch
				curHeap = s.mTextureHeap;
				curSRV = s.mTextureSRV;
				curq = ctx->mQuadOffset;
			}

			ctx->AddQuad(s.mPixelRect, s.mTextureRect, s.mColor);
		}

		if (curHeap && curSRV.ptr) ctx->DrawQuadGroup(commandList, curq, curHeap, curSRV, screenMVP);

		mQuadDrawQueueCount = 0;
	}

	// Draw lines
	unsigned int lsize = 0;
	for (unsigned int i = 0; i < mLineDrawQueueCount; i++) lsize += ((unsigned int)mLineDrawQueue[i].mVertices.size() - 1) * 2;
	if (lsize > 0) {
		ctx->ResizeLines(lsize);

		if (!mColoredShader) CreateShader();
		commandList->SetMaterial(nullptr);
		commandList->SetShader(mColoredShader);
		commandList->SetBlendState(BLEND_STATE_ALPHA);
		commandList->SetCullMode(D3D12_CULL_MODE_NONE);
		commandList->DrawUserMesh((MESH_SEMANTIC)(MESH_SEMANTIC_POSITION | MESH_SEMANTIC_COLOR0), D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

		unsigned int curl = ctx->mLineIndexOffset;
		for (unsigned int i = 0; i < mLineDrawQueueCount; i++)
			ctx->AddLines(mLineDrawQueue[i]);

		ctx->DrawLines(commandList->D3DCommandList(), curl, screenMVP);

		mLineDrawQueueCount = 0;
	}
}