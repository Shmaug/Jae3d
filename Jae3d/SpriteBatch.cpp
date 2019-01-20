#include "SpriteBatch.hpp"

#include <wrl.h>
#include "Graphics.hpp"
#include "CommandList.hpp"
#include "AssetDatabase.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Camera.hpp"

using namespace Microsoft::WRL;
using namespace DirectX;

SpriteBatch::~SpriteBatch() { Release(); }

void SpriteBatch::Release() {
	for (int i = 0; i < mContexts.size(); i++)
		delete mContexts[i];
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
void SpriteBatch::SpriteContext::ResizeQuads(size_t length) {
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
void SpriteBatch::SpriteContext::ResizeLines(size_t length) {
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
}

void SpriteBatch::DrawLines(jvector<DirectX::XMFLOAT3> vertices, jvector<DirectX::XMFLOAT4> colors) {
	mLineDrawQueue.push_back({ vertices, colors });
}

XMFLOAT2 MeasureText(std::shared_ptr<Font> font, float scale, jwstring text) {
	float w = (float)font->GetTexture()->Width();
	float h = (float)font->GetTexture()->Height();
	scale *= (float)Graphics::GetWindow()->GetLogPixelsX() / font->GetTextureDpi();
	XMFLOAT2 m(0, 0);
	XMFLOAT2 p(0, 0);
	FontGlyph g;
	wchar_t prev = L'\0';
	for (unsigned int j = 0; j < text.length(); j++) {
		wchar_t cur = text[j];
		if (cur == L'\n') {
			p.x = 0;
			p.y += scale * font->GetLineSpacing();
		} else {
			if (font->HasGlyph(cur))
				g = font->GetGlyph(cur);
			else if (font->HasGlyph(L'?'))
				g = font->GetGlyph(L'?');
			else {
				prev = cur;
				continue;
			}

			p.x += scale * font->GetKerning(prev, cur);
			p.x += scale * g.advance;
		}
		m.x = fmax(m.x, p.x);
		m.y = fmax(m.y, p.y);
		prev = cur;
	}
	return m;
}

void SpriteBatch::DrawText (std::shared_ptr<Font> font, XMFLOAT2 pos, float scale, XMFLOAT4 color, jwstring text) {
	// TODO: text rendering is slow

	float w = (float)font->GetTexture()->Width();
	float h = (float)font->GetTexture()->Height();
	scale *= (float)Graphics::GetWindow()->GetLogPixelsX() / font->GetTextureDpi();
	XMFLOAT2 p = pos;
	FontGlyph g;
	wchar_t prev = L'\0';
	for (unsigned int j = 0; j < text.length(); j++) {
		wchar_t cur = text[j];
		if (cur == L'\n') {
			p.x = pos.x;
			p.y += scale * font->GetLineSpacing();
		} else {
			if (font->HasGlyph(cur))
				g = font->GetGlyph(cur);
			else if (font->HasGlyph(L'?'))
				g = font->GetGlyph(L'?');
			else {
				prev = cur;
				continue;
			}

			p.x += scale * font->GetKerning(prev, cur);

			if (g.character != L' ') {
				mQuadDrawQueue.push_back(
					SpriteDraw(font->GetTexture()->GetSRVDescriptorHeap(), font->GetTexture()->GetSRVGPUDescriptor(),
						XMFLOAT4(
						p.x + scale * g.ox,
						p.y - scale * g.oy,
						p.x + scale * (g.tw + g.ox),
						p.y + scale * (g.th - g.oy)),
						XMFLOAT4((float)g.tx0 / w, (float)g.ty0 / h, (float)(g.tx0 + g.tw) / w, (float)(g.ty0 + g.th) / h),
						color)
				);
			}

			p.x += scale * g.advance;
		}
		prev = cur;
	}
}
void SpriteBatch::DrawTextf(std::shared_ptr<Font> font, XMFLOAT2 pos, float scale, XMFLOAT4 color, jwstring fmt, ...) {
	wchar_t buf[1025];
	va_list args;
	va_start(args, fmt);
	wvsprintf(buf, fmt.c_str(), args);
	va_end(args);
	DrawText(font, pos, scale, color, jwstring(buf));
}

void SpriteBatch::DrawTexture(ComPtr<ID3D12DescriptorHeap> mTextureHeap, D3D12_GPU_DESCRIPTOR_HANDLE mTextureSRV, XMFLOAT4 rect, XMFLOAT4 color, XMFLOAT4 srcRect) {
	mQuadDrawQueue.push_back(SpriteDraw(mTextureHeap, mTextureSRV, rect, srcRect, color));
}
void SpriteBatch::DrawTexture(std::shared_ptr<Texture> texture, XMFLOAT4 rect, XMFLOAT4 color, XMFLOAT4 srcRect) {
	mQuadDrawQueue.push_back(SpriteDraw(texture->GetSRVDescriptorHeap(), texture->GetSRVGPUDescriptor(), rect, srcRect, color));
}

SpriteBatch::SpriteContext* SpriteBatch::GetContext(unsigned int frameIndex) {
	if (frameIndex < mContexts.size()) {
		mContexts[frameIndex]->Reset();
		return mContexts[frameIndex];
	}
	// create contexts for each frame
	while (mContexts.size() < frameIndex + 1)
		mContexts.push_back(new SpriteBatch::SpriteContext());
	return mContexts[frameIndex];
}

void SpriteBatch::SpriteContext::AddQuad(XMFLOAT4 screenRect, XMFLOAT4 texRect, XMFLOAT4 color) {
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
void SpriteBatch::SpriteContext::AddLines(LineDraw &lines) {
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

void SpriteBatch::SpriteContext::DrawQuadGroup(std::shared_ptr<CommandList> commandList, unsigned int startQuad, ComPtr<ID3D12DescriptorHeap> heap, D3D12_GPU_DESCRIPTOR_HANDLE tex) {
	auto cmdList = commandList->D3DCommandList();

	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, XMMatrixOrthographicOffCenterLH(0, (float)commandList->CurrentRenderTargetWidth(), (float)commandList->CurrentRenderTargetHeight(), 0, 0, 1.0f));
	cmdList->SetGraphicsRoot32BitConstants(0, 16, &m, 0);

	ID3D12DescriptorHeap* heaps = { heap.Get() };
	cmdList->SetDescriptorHeaps(1, &heaps);
	cmdList->SetGraphicsRootDescriptorTable(1, tex);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &mQuadVertexBufferView);
	cmdList->IASetIndexBuffer(&mQuadIndexBufferView);
	cmdList->DrawIndexedInstanced((mQuadOffset - startQuad) * 6, 1, startQuad * 6, 0, 0);
}
void SpriteBatch::SpriteContext::DrawLines(ComPtr<ID3D12GraphicsCommandList> cmdList, unsigned int startIndex) {
	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, XMMatrixOrthographicOffCenterLH(0, (float)Graphics::GetWindow()->GetWidth(), (float)Graphics::GetWindow()->GetHeight(), 0, 0, 1.0f));
	cmdList->SetGraphicsRoot32BitConstants(0, 16, &m, 0);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	cmdList->IASetVertexBuffers(0, 1, &mLineVertexBufferView);
	cmdList->IASetIndexBuffer(&mLineIndexBufferView);
	cmdList->DrawIndexedInstanced(mLineIndexOffset - startIndex, 1, startIndex, 0, 0);
}

void SpriteBatch::Flush(std::shared_ptr<CommandList> commandList){
	// Draw quads
	size_t qsize = mQuadDrawQueue.size();
	if (qsize > 0) {
		SpriteContext* ctx = GetContext(commandList->GetFrameIndex());
		ctx->ResizeQuads(qsize);

		unsigned int curq = 0;
		ComPtr<ID3D12DescriptorHeap> curHeap = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE curSRV = { 0 };

		if (!mTexturedShader) CreateShader();
		commandList->SetShader(mTexturedShader);
		commandList->SetBlendState(BLEND_STATE_ALPHA);
		commandList->DrawUserMesh((MESH_SEMANTIC)(MESH_SEMANTIC_POSITION | MESH_SEMANTIC_TEXCOORD0 | MESH_SEMANTIC_COLOR0), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		for (unsigned int i = 0; i < mQuadDrawQueue.size(); i++) {
			SpriteDraw s = mQuadDrawQueue[i];

			if (curSRV.ptr != s.mTextureSRV.ptr) {
				// Draw previous batch
				if (curHeap && curSRV.ptr) ctx->DrawQuadGroup(commandList, curq, curHeap, curSRV);
				// Start new batch
				curHeap = s.mTextureHeap;
				curSRV = s.mTextureSRV;
				curq = ctx->mQuadOffset;
			}

			ctx->AddQuad(s.mPixelRect, s.mTextureRect, s.mColor);
		}

		if (curHeap && curSRV.ptr) ctx->DrawQuadGroup(commandList, curq, curHeap, curSRV);

		mQuadDrawQueue.clear();
	}

	// Draw lines
	size_t lsize = 0;
	for (unsigned int i = 0; i < mLineDrawQueue.size(); i++) lsize += (mLineDrawQueue[i].mVertices.size() - 1) * 2;
	if (lsize > 0) {
		SpriteContext* ctx = GetContext(commandList->GetFrameIndex());
		ctx->ResizeLines(lsize);

		if (!mColoredShader) CreateShader();
		commandList->SetShader(mColoredShader);
		commandList->SetBlendState(BLEND_STATE_ALPHA);
		commandList->DrawUserMesh((MESH_SEMANTIC)(MESH_SEMANTIC_POSITION | MESH_SEMANTIC_COLOR0), D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

		unsigned int curl = ctx->mLineIndexOffset;
		for (unsigned int i = 0; i < mLineDrawQueue.size(); i++)
			ctx->AddLines(mLineDrawQueue[i]);

		ctx->DrawLines(commandList->D3DCommandList(), curl);

		mLineDrawQueue.clear();
	}
}