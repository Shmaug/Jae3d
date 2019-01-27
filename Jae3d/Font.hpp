#pragma once

#include "Common.hpp"

#include "Asset.hpp"
#include "Texture.hpp"
#include "MemoryStream.hpp"

class Font : public Asset {
public:
	JAE_API Font(jwstring name);
	JAE_API Font(jwstring name,
		unsigned int size,
		unsigned int height,
		unsigned int ascender,
		unsigned int descender,
		unsigned int lineSpacing,
		unsigned int texDpi,
		std::shared_ptr<Texture> tex,
		jvector<FontGlyph>& glyphs, jvector<FontKerning>& kernings);
	JAE_API Font(jwstring name, MemoryStream &ms);
	JAE_API ~Font();

	JAE_API void WriteData(MemoryStream &ms) override;
	JAE_API uint64_t TypeId() override;

	JAE_API std::shared_ptr<Texture> GetTexture() const { return mTexture; };

	JAE_API int GetKerning(const wchar_t from, const wchar_t to) const;
	JAE_API bool GetGlyph(const wchar_t c, FontGlyph& g) const;

	unsigned int GetSize() const { return mSize; };
	unsigned int GetLineSpacing() const { return mLineSpace; };
	unsigned int GetHeight() const { return mHeight; };
	unsigned int GetAscender() const { return mAscender; };
	unsigned int GetDescender() const { return mDescender; };
	unsigned int GetTextureDpi() const { return mTexDpi; }

private:
	unsigned int mSize;
	unsigned int mHeight;
	unsigned int mAscender;
	unsigned int mDescender;
	unsigned int mLineSpace;
	unsigned int mTexDpi;

	// TODO: something more memory efficient?
	jvector<FontGlyph> mGlyphs;
	jvector<unsigned int> mGlyphIndices;
	std::unordered_map<uint32_t, int> mKerning;
	std::shared_ptr<Texture> mTexture;
};

