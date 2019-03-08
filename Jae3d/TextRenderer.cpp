#include "TextRenderer.hpp"

#include "Graphics.hpp"
#include "CommandList.hpp"
#include "ConstantBuffer.hpp"
#include "Material.hpp"
#include "Font.hpp"
#include "Material.hpp"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

TextRenderer::TextRenderer() : TextRenderer(L"") {}
TextRenderer::TextRenderer(const jwstring& name) : Renderer(name), mVisible(true), mTextScale(.01f) {
	mCBuffer = shared_ptr<ConstantBuffer>(new ConstantBuffer(sizeof(XMFLOAT4X4) * 2, L"MeshRenderer CB", Graphics::BufferCount()));
	mMesh = new TextMesh[Graphics::BufferCount()];
}
TextRenderer::~TextRenderer() { delete[] mMesh; }

void TextRenderer::AddQuad(jvector<TextVertex>& vertices, jvector<uint16_t>& indices, XMFLOAT4 rect, XMFLOAT4 uvRect) {
	unsigned int v = (unsigned int)vertices.size();

	indices.push_back(v + 0);
	indices.push_back(v + 1);
	indices.push_back(v + 2);
	indices.push_back(v + 1);
	indices.push_back(v + 3);
	indices.push_back(v + 2);

	rect.x *= mTextScale;
	rect.y *= mTextScale;
	rect.z *= mTextScale;
	rect.w *= mTextScale;

	vertices.push_back(TextVertex(rect.x, rect.y, uvRect.x, uvRect.y));
	vertices.push_back(TextVertex(rect.z, rect.y, uvRect.z, uvRect.y));
	vertices.push_back(TextVertex(rect.x, rect.w, uvRect.x, uvRect.w));
	vertices.push_back(TextVertex(rect.z, rect.w, uvRect.z, uvRect.w));
}

void TextRenderer::Build(TextMesh& m) {
	if (!mFont) return;

	jvector<TextVertex> vertices;
	jvector<uint16_t> indices;

	float w = (float)mFont->GetTexture()->Width();
	float h = (float)mFont->GetTexture()->Height();

	XMFLOAT2 p{ 0,0 };
	wchar_t prev = L'\0';
	FontGlyph g;
	for (unsigned int j = 0; j < mText.length(); j++) {
		wchar_t cur = mText[j];
		if (cur == L'\n') {
			p.x = 0;
			p.y += mFont->GetLineSpacing();
		} else {
			if (!mFont->GetGlyph(cur, g) && !mFont->GetGlyph(L'?', g)) {
				prev = cur;
				continue;
			}
			p.x += mFont->GetKerning(prev, cur);

			if (g.character != L' ') {
				AddQuad(vertices, indices,
					XMFLOAT4(
						p.x + g.ox,
						p.y + g.oy,
						p.x + (g.tw + g.ox),
						p.y - g.th + g.oy),
					XMFLOAT4(
						(float)g.tx0 / w,
						(float)g.ty0 / h,
						(float)(g.tx0 + g.tw) / w,
						(float)(g.ty0 + g.th) / h ) );
			}

			p.x += g.advance;
		}
		prev = cur;
	}

	XMFLOAT3 min = vertices[0].position;
	XMFLOAT3 max = vertices[0].position;
	for (unsigned int i = 1; i < vertices.size(); i++) {
		min.x = fmin(min.x, vertices[i].position.x);
		min.y = fmin(min.y, vertices[i].position.y);
		min.z = fmin(min.z, vertices[i].position.z);
		max.x = fmax(min.x, vertices[i].position.x);
		max.y = fmax(min.y, vertices[i].position.y);
		max.z = fmax(min.z, vertices[i].position.z);
	}
	mBounds = BoundingBox(
		XMFLOAT3((min.x + max.x) * .5f, (min.y + max.y) * .5f, (min.z + max.z) * .5f),
		XMFLOAT3((max.x - min.x) * .5f, (max.y - min.y) * .5f, (max.z - min.z) * .5f));
	mBounds.Extents.z = .01f;

	auto device = Graphics::GetDevice();

	if (m.mMappedLength < vertices.size() / 4) {
		// Vertices
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(TextVertex)),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&m.mVertexBuffer)));

		m.mVertexBuffer->SetName(L"Quad Vertex Buffer");

		void* vdata;
		CD3DX12_RANGE readRange(0, 0);
		m.mVertexBuffer->Map(0, &readRange, &vdata);

		m.mVertexBufferView.BufferLocation = m.mVertexBuffer->GetGPUVirtualAddress();
		m.mVertexBufferView.SizeInBytes = (UINT)vertices.size() * sizeof(TextVertex);
		m.mVertexBufferView.StrideInBytes = sizeof(TextVertex);

		m.mMappedVertices = reinterpret_cast<TextVertex*>(vdata);

		// Indices
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(uint16_t)),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&m.mIndexBuffer)));

		m.mIndexBuffer->SetName(L"Quad Index Buffer");

		void* idata;
		m.mIndexBuffer->Map(0, &readRange, &idata);

		m.mIndexBufferView.BufferLocation = m.mIndexBuffer->GetGPUVirtualAddress();
		m.mIndexBufferView.SizeInBytes = (UINT)indices.size() * sizeof(uint16_t);
		m.mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;

		m.mMappedIndices = reinterpret_cast<uint16_t*>(idata);

		m.mMappedLength = (unsigned int)vertices.size() / 4;
	}
	m.mQuadCount = (unsigned int)vertices.size() / 4;

	memcpy(m.mMappedVertices, vertices.data(), vertices.size() * sizeof(TextVertex));
	memcpy(m.mMappedIndices, indices.data(), indices.size() * sizeof(uint16_t));
}

BoundingOrientedBox TextRenderer::Bounds() {
	XMFLOAT3 mcenter = mBounds.Center;
	XMFLOAT3 mbounds = mBounds.Extents;
	XMFLOAT3 scale = WorldScale();
	XMVECTOR p = XMVector3Transform(XMLoadFloat3(&mcenter), XMLoadFloat4x4(&ObjectToWorld()));
	XMStoreFloat3(&mcenter, p);
	return BoundingOrientedBox(mcenter, XMFLOAT3(mbounds.x * scale.x, mbounds.y * scale.y, mbounds.z * scale.z), WorldRotation());
}

bool TextRenderer::TextRenderJob::LessThan(RenderJob* b) {
	if (RenderJob::LessThan(b)) return true;
	return false;
	TextRenderJob* t = dynamic_cast<TextRenderJob*>(b);
	if (!t) return false;
	return t->mMaterial->mName < mMaterial->mName;
}
void TextRenderer::TextRenderJob::Execute(const shared_ptr<CommandList>& commandList, const std::shared_ptr<::Material>& materialOverride) {
	if (materialOverride) mMaterial = materialOverride;
	mMaterial->SetCBuffer("ObjectBuffer", mObjectBuffer, commandList->GetFrameIndex());
	commandList->SetMaterial(mMaterial);
	commandList->SetBlendState(BLEND_STATE_ALPHA);
	commandList->SetCullMode(D3D12_CULL_MODE_NONE);
	commandList->DrawUserMesh((MESH_SEMANTIC)(MESH_SEMANTIC_NORMAL | MESH_SEMANTIC_TANGENT | MESH_SEMANTIC_BINORMAL | MESH_SEMANTIC_TEXCOORD0));
	commandList->Draw(mMesh.mVertexBufferView, mMesh.mIndexBufferView, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, mMesh.mQuadCount * 6, 0, 0);
}

void TextRenderer::GatherRenderJobs(const shared_ptr<CommandList>& commandList, const shared_ptr<Camera>& camera, jvector<RenderJob*>& list) {
	if (!mMaterial || !mFont || mText.empty()) return;
	UpdateTransform();

	mCBuffer->WriteFloat4x4(ObjectToWorld(), 0, commandList->GetFrameIndex());
	mCBuffer->WriteFloat4x4(WorldToObject(), sizeof(XMFLOAT4X4), commandList->GetFrameIndex());

	TextMesh& m = mMesh[commandList->GetFrameIndex()];
	if (m.mMaterial != mMaterial) {
		m.mMaterial = mMaterial;
		if (m.mFont && m.mFont == mFont) m.mMaterial->SetTexture("Texture", mFont->GetTexture(), -1);
	}
	if (m.mFont != mFont) {
		m.mFont = mFont;
		if (m.mMaterial) m.mMaterial->SetTexture("Texture", mFont->GetTexture(), -1);
	}
	if (m.mText != mText) Build(m);

	if (!m.mMappedLength) return;

	list.push_back(new TextRenderJob(mMaterial->RenderQueue(), m, mMaterial, mCBuffer));
}