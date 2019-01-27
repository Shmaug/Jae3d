#include "Font.hpp"
#include "AssetFile.hpp"

#include "Graphics.hpp"
#include "CommandList.hpp"
#include "d3dx12.hpp"

using namespace DirectX;

Font::Font(jwstring name) : Asset(name) {}
Font::Font(jwstring name, MemoryStream &ms) : Asset(name, ms) {
	mTexture = std::shared_ptr<Texture>(new Texture(name, ms));
	mTexDpi = ms.Read<uint32_t>();
	mSize = ms.Read<uint32_t>();
	mHeight = ms.Read<uint32_t>();
	mLineSpace = ms.Read<uint32_t>();
	mAscender = ms.Read<uint32_t>();
	mDescender = ms.Read<uint32_t>();

	mGlyphIndices.resize(0xFFFF);

	uint32_t c = ms.Read<uint32_t>();
	mGlyphs.reserve(c);

	for (unsigned int i = 0; i < c; i++) {
		FontGlyph g;
		g.character = (wchar_t)ms.Read<int16_t>();
		g.advance = ms.Read<uint32_t>();
		g.ox = ms.Read<uint32_t>();
		g.oy = ms.Read<uint32_t>();
		g.tx0 = ms.Read<uint32_t>();
		g.ty0 = ms.Read<uint32_t>();
		g.tw = ms.Read<uint32_t>();
		g.th = ms.Read<uint32_t>();
		mGlyphIndices[g.character] = (unsigned int)mGlyphs.size();
		mGlyphs.push_back(g);
	}

	uint32_t kc = ms.Read<uint32_t>();
	mKerning = std::unordered_map<uint32_t, int>(kc);

	for (unsigned int i = 0; i < kc; i++) {
		uint32_t k = ms.Read<uint32_t>();
		int32_t kv = ms.Read<int32_t>();
		mKerning.emplace(k, kv);
	}
}

Font::Font(jwstring name,
	unsigned int size,
	unsigned int height,
	unsigned int ascender,
	unsigned int descender,
	unsigned int lineSpacing,
	unsigned int texDpi,
	std::shared_ptr<Texture> tex,
	jvector<FontGlyph>& glyphs, jvector<FontKerning>& kernings)
	: Asset(name), mSize(size), mHeight(height), mAscender(ascender), mDescender(descender), mLineSpace(lineSpacing),
	mTexDpi(texDpi), mTexture(tex)
{
	mGlyphIndices.resize(0xFFFF);
	mGlyphs = glyphs;

	for (unsigned int i = 0; i < mGlyphs.size(); i++)
		mGlyphIndices[mGlyphs[i].character] = i;

	mKerning = std::unordered_map<uint32_t, int>(kernings.size());
	for (int i = 0; i < kernings.size(); i++)
		mKerning.emplace((unsigned int)kernings[i].to | ((unsigned int)kernings[i].from << 16), kernings[i].offset);
}
Font::~Font() {}
uint64_t Font::TypeId() { return ASSET_TYPE_FONT; }

bool Font::GetGlyph(const wchar_t c, FontGlyph &g) const {
	if (mGlyphs[mGlyphIndices[c]].character) {
		g = mGlyphs[mGlyphIndices[c]];
		return true;
	}
	return false;
}

int Font::GetKerning(const wchar_t from, const wchar_t to) const {
	uint32_t key = (unsigned int)to | ((unsigned int)from << 16);
	if (mKerning.count(key))
		return mKerning.at(key);
	return 0;
}

void Font::WriteData(MemoryStream &ms) {
	mTexture->WriteData(ms);
	ms.Write((uint32_t)mTexDpi);
	ms.Write((uint32_t)mSize);
	ms.Write((uint32_t)mHeight);
	ms.Write((uint32_t)mLineSpace);
	ms.Write((uint32_t)mAscender);
	ms.Write((uint32_t)mDescender);

	size_t pos = ms.Tell();
	ms.Write((uint32_t)0);
	if (!mGlyphs.empty()) {
		unsigned int i = 0;
		for (const auto &it : mGlyphs) {
			ms.Write((int16_t) it.character);
			ms.Write((uint32_t)it.advance);
			ms.Write((uint32_t)it.ox);
			ms.Write((uint32_t)it.oy);
			ms.Write((uint32_t)it.tx0);
			ms.Write((uint32_t)it.ty0);
			ms.Write((uint32_t)it.tw);
			ms.Write((uint32_t)it.th);
			i++;
		}
		size_t posc = ms.Tell();
		ms.Seek(pos);
		ms.Write((uint32_t)i);
		ms.Seek(posc);
	}

	pos = ms.Tell();
	ms.Write((uint32_t)0);
	if (!mKerning.empty()) {
		int i = 0;
		for (const auto &it : mKerning) {
			ms.Write((uint32_t)it.first);
			ms.Write((int32_t)it.second);
			i++;
		}
		size_t posc = ms.Tell();
		ms.Seek(pos);
		ms.Write((uint32_t)i);
		ms.Seek(posc);
	}
}