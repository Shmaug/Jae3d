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

	uint32_t c = ms.Read<uint32_t>();
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
		mGlyphs.emplace(g.character, g);
	}

	uint32_t kc = ms.Read<uint32_t>();
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
	jvector<FontGlyph> glyphs, jvector<FontKerning> kernings)
	: Asset(name), mSize(size), mHeight(height), mAscender(ascender), mDescender(descender), mLineSpace(lineSpacing), mTexDpi(texDpi), mTexture(tex)
{
	for (int i = 0; i < glyphs.size(); i++)
		mGlyphs.emplace(glyphs[i].character, glyphs[i]);

	for (int i = 0; i < kernings.size(); i++) {
		unsigned int key = (unsigned int)kernings[i].to | ((unsigned int)kernings[i].from << 16);
		mKerning.emplace(key, kernings[i].offset);
	}
}
Font::~Font() {}
uint64_t Font::TypeId() { return AssetFile::TYPEID_FONT; }

FontGlyph Font::GetGlyph(wchar_t c) const {
	return mGlyphs.at(c);
}
bool Font::HasGlyph(wchar_t c) const {
	return mGlyphs.has(c);
}

int Font::GetKerning(wchar_t from, wchar_t to) const {
	unsigned int key = (unsigned int)to | ((unsigned int)from << 16);
	if (mKerning.has(key))
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
		int i = 0;
		auto it = mGlyphs.begin();
		while (it.Valid()) {
			ms.Write((int16_t)(*it).Value().character);
			ms.Write((uint32_t)(*it).Value().advance);
			ms.Write((uint32_t)(*it).Value().ox);
			ms.Write((uint32_t)(*it).Value().oy);
			ms.Write((uint32_t)(*it).Value().tx0);
			ms.Write((uint32_t)(*it).Value().ty0);
			ms.Write((uint32_t)(*it).Value().tw);
			ms.Write((uint32_t)(*it).Value().th);
			it++;
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
		auto it = mKerning.begin();
		while (it.Valid()) {
			ms.Write((uint32_t)(*it).Key());
			ms.Write((int32_t)(*it).Value());
			it++;
			i++;
		}
		size_t posc = ms.Tell();
		ms.Seek(pos);
		ms.Write((uint32_t)i);
		ms.Seek(posc);
	}
}