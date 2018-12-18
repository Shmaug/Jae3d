#include "SpriteBatch.hpp"

#include <wrl.h>
#include "Graphics.hpp"
#include "CommandList.hpp"
#include "Shader.hpp"
#include "Window.hpp"

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
void SpriteBatch::DrawText (std::shared_ptr<Font> font, XMFLOAT2 pos, float scale, XMFLOAT4 color, jwstring text) {
	float w = (float)font->GetTexture()->Width();
	float h = (float)font->GetTexture()->Height();
	scale *= (float)Graphics::GetWindow()->GetLogPixelsX() / font->GetTextureDpi();
	XMFLOAT2 p = pos;
	wchar_t prev = L'\0';
	for (unsigned int j = 0; j < text.length(); j++) {
		wchar_t cur = text[j];
		if (cur == L'\n') {
			p.x = pos.x;
			p.y += scale * font->GetLineSpacing();
		} else {
			FontGlyph g;
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
				SpriteDraw sprite(font->GetTexture(),
					XMFLOAT4(
					p.x + scale * g.ox,
					p.y - scale * g.oy,
					p.x + scale * (g.tw + g.ox),
					p.y + scale * (g.th - g.oy)),
					XMFLOAT4((float)g.tx0 / w, (float)g.ty0 / h, (float)(g.tx0 + g.tw) / w, (float)(g.ty0 + g.th) / h),
					color);
				mQuadDrawQueue.push_back(sprite);
			}

			p.x += scale * g.advance;
		}
		prev = cur;
	}
}
void SpriteBatch::DrawText (std::shared_ptr<Font> font, XMFLOAT2 pos, float scale, XMFLOAT4 color, const wchar_t* text) {
	DrawText(font, pos, scale, color, jwstring(text));
}
void SpriteBatch::DrawTextf(std::shared_ptr<Font> font, XMFLOAT2 pos, float scale, XMFLOAT4 color, jwstring fmt, ...) {
	wchar_t buf[1025];
	va_list args;
	va_start(args, fmt);
	wvsprintf(buf, fmt.c_str(), args);
	va_end(args);
	DrawText(font, pos, scale, color, jwstring(buf));
}

void SpriteBatch::DrawText (std::shared_ptr<Font> font, XMFLOAT4 rect, float scale, XMFLOAT4 color, jwstring text) {
	float w = (float)font->GetTexture()->Width();
	float h = (float)font->GetTexture()->Height();
	scale *= (float)Graphics::GetWindow()->GetLogPixelsX() / font->GetTextureDpi();
	XMFLOAT2 p(rect.x, rect.y);
	wchar_t prev = L'\0';
	for (unsigned int j = 0; j < text.length(); j++) {
		wchar_t cur = text[j];
		if (cur == L'\n') {
			p.x = rect.x;
			p.y += scale * font->GetLineSpacing();
		} else {
			FontGlyph g;
			if (font->HasGlyph(cur))
				g = font->GetGlyph(cur);
			else if (font->HasGlyph(L'?'))
				g = font->GetGlyph(L'?');
			else {
				prev = cur;
				continue;
			}

			p.x += scale * font->GetKerning(prev, cur);

			if (p.x + scale * (g.tw + g.ox) > rect.z) {
				p.x = rect.x;
				p.y += scale * font->GetHeight();
			}

			if (g.character != L' ') {
				SpriteDraw sprite(font->GetTexture(),
					XMFLOAT4(
						p.x + scale * g.ox,
						p.y - scale * g.oy,
						p.x + scale * (g.tw + g.ox),
						p.y + scale * (g.th - g.oy)),
					XMFLOAT4((float)g.tx0 / w, (float)g.ty0 / h, (float)(g.tx0 + g.tw) / w, (float)(g.ty0 + g.th) / h),
					color);
				mQuadDrawQueue.push_back(sprite);
			}

			p.x += scale * g.advance;
		}
		prev = cur;
	}
}
void SpriteBatch::DrawText (std::shared_ptr<Font> font, XMFLOAT4 rect, float scale, XMFLOAT4 color, const wchar_t* text) {
	DrawText(font, rect, scale, color, jwstring(text));
}
void SpriteBatch::DrawTextf(std::shared_ptr<Font> font, XMFLOAT4 rect, float scale, XMFLOAT4 color, jwstring fmt, ...) {
	wchar_t buf[1025];
	va_list args;
	va_start(args, fmt);
	wvsprintf(buf, fmt.c_str(), args);
	va_end(args);
	DrawText(font, rect, scale, color, jwstring(buf));
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
	const char texrootsig[] =
" #define RootSig "
" \"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS ),"
" RootConstants(num32BitConstants=16, b0, visibility=SHADER_VISIBILITY_VERTEX),"
" DescriptorTable(SRV(t0), visibility=SHADER_VISIBILITY_PIXEL),"
" StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_LINEAR, visibility=SHADER_VISIBILITY_PIXEL)\""
;
	const char texshader[] =
" struct CB { float4x4 proj; }; "
" ConstantBuffer<CB> C : register(b0); "
" Texture2D<float4> Tex : register(t0); "
" sampler Samp : register(s0); "
" struct v2f { float4 pos : SV_Position; float4 color : COLOR0; float2 tex : TEXCOORD0; };"
" v2f vsmain(float3 vertex : POSITION, float4 color : COLOR0, float2 tex : TEXCOORD0) {"
"	v2f o;"
"	o.pos = mul(C.proj, float4(vertex, 1));"
"	o.tex = tex;"
"	o.color = color;"
"	return o;"
" }"
" float4 psmain(float4 pos : SV_Position, float4 color : COLOR0, float2 tex : TEXCOORD0) : SV_Target {"
" 	return Tex.Sample(Samp, tex) * color;"
" }"
;
	
	mTexturedShader = std::shared_ptr<Shader>(new Shader(L"Textured Sprite Shader"));
	mTexturedShader->CompileShaderStage(texrootsig, "RootSig", SHADER_STAGE_ROOTSIG);
	mTexturedShader->CompileShaderStage(texshader, "vsmain", SHADER_STAGE_VERTEX);
	mTexturedShader->CompileShaderStage(texshader, "psmain", SHADER_STAGE_PIXEL);
	mTexturedShader->Upload();

		const char colrootsig[] =
" #define RootSig "
" \"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS ),"
" RootConstants(num32BitConstants=16, b0, visibility=SHADER_VISIBILITY_VERTEX)\""
;
	const char colshader[] =
" struct CB { float4x4 proj; }; "
" ConstantBuffer<CB> C : register(b0); "
" struct v2f { float4 pos : SV_Position; float4 color : COLOR0; };"
" v2f vsmain(float3 vertex : POSITION, float4 color : COLOR0) {"
"	v2f o;"
"	o.pos = mul(C.proj, float4(vertex, 1));"
"	o.color = color;"
"	return o;"
" }"
" float4 psmain(float4 pos : SV_Position, float4 color : COLOR0) : SV_Target {"
" 	return color;"
" }"
;
	
	mColoredShader = std::shared_ptr<Shader>(new Shader(L"Colored Sprite Shader"));
	mColoredShader->CompileShaderStage(colrootsig, "RootSig", SHADER_STAGE_ROOTSIG);
	mColoredShader->CompileShaderStage(colshader, "vsmain", SHADER_STAGE_VERTEX);
	mColoredShader->CompileShaderStage(colshader, "psmain", SHADER_STAGE_PIXEL);
	mColoredShader->Upload();
}

void SpriteBatch::SpriteContext::DrawQuadGroup(ComPtr<ID3D12GraphicsCommandList> cmdList, std::shared_ptr<Texture> tex, unsigned int startQuad) {
	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, XMMatrixOrthographicOffCenterLH(0, (float)Graphics::GetWindow()->GetWidth(), (float)Graphics::GetWindow()->GetHeight(), 0, 0, 1.0f));
	cmdList->SetGraphicsRoot32BitConstants(0, 16, &m, 0);

	ID3D12DescriptorHeap* heap = { tex->GetSRVDescriptorHeap().Get() };
	cmdList->SetDescriptorHeaps(1, &heap);
	cmdList->SetGraphicsRootDescriptorTable(1, tex->GetSRVGPUDescriptor());

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
		std::shared_ptr<Texture> curTex;

		if (!mTexturedShader) CreateShader();
		commandList->SetCamera(nullptr);
		commandList->SetShader(mTexturedShader);
		commandList->SetBlendState(BLEND_STATE_ALPHA);
		commandList->DrawUserMesh((MESH_SEMANTIC)(MESH_SEMANTIC_POSITION | MESH_SEMANTIC_TEXCOORD0 | MESH_SEMANTIC_COLOR0), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		for (unsigned int i = 0; i < mQuadDrawQueue.size(); i++) {
			SpriteDraw s = mQuadDrawQueue[i];

			if (curTex != s.mTexture) {
				// Draw previous batch
				if (curTex) ctx->DrawQuadGroup(commandList->D3DCommandList(), curTex, curq);
				curTex = s.mTexture;
				curq = ctx->mQuadOffset;
			}

			ctx->AddQuad(s.mPixelRect, s.mTextureRect, s.mColor);
		}

		if (curTex) ctx->DrawQuadGroup(commandList->D3DCommandList(), curTex, curq);

		mQuadDrawQueue.clear();
	}

	// Draw lines
	size_t lsize = 0;
	for (unsigned int i = 0; i < mLineDrawQueue.size(); i++) lsize += (mLineDrawQueue[i].mVertices.size() - 1) * 2;
	if (lsize > 0) {
		SpriteContext* ctx = GetContext(commandList->GetFrameIndex());
		ctx->ResizeLines(lsize);

		if (!mColoredShader) CreateShader();
		commandList->SetCamera(nullptr);
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