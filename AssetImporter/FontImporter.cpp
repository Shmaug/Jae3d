#include "FontImporter.hpp"
#include "TextureImporter.hpp"

#include <IOUtil.hpp>

#include <ft2build.h>
#include <freetype/ftbitmap.h>
#include FT_FREETYPE_H
#define FT_FLOOR(X) (((X) & -64) / 64)
#define FT_CEIL(X)  ((((X) + 63) & -64) / 64)

#include <string>
#include <cwctype>
#include <fstream>

using namespace std;

void ImportFont(jwstring path, jvector<AssetMetadata> &meta) {
	AssetMetadata metadata(path);
	metadata.asset = shared_ptr<Asset>(ImportFont(path, metadata));
	meta.push_back(metadata);
}

Font* ImportFont(jwstring path) {
	AssetMetadata metadata(path);
	return ImportFont(path, metadata);
}
Font* ImportFont(jwstring path, AssetMetadata &meta) {
	FT_Library library;
	auto err = FT_Init_FreeType(&library);
	if (err) {
		cerr << L"Failed to initialize FreeType!\n";
		FT_Done_FreeType(library);
		return nullptr;
	}

	FT_Face face;
	err = FT_New_Face(library, utf16toUtf8(path).c_str(), 0, &face);
	if (err) {
		cerr << L"Failed to read font file!\n";
		FT_Done_FreeType(library);
		return nullptr;
	}

	FT_CharMap found = nullptr;
	for (auto i = 0; i < face->num_charmaps; i++) {
		const auto charmap = face->charmaps[i];
		if ((charmap->platform_id == 3 && charmap->encoding_id == 1) /* Windows Unicode */
			|| (charmap->platform_id == 3 && charmap->encoding_id == 0) /* Windows Symbol */
			|| (charmap->platform_id == 2 && charmap->encoding_id == 1) /* ISO Unicode */
			|| (charmap->platform_id == 0)) { /* Apple Unicode */
			found = charmap;
			break;
		}
	}
	if (found) FT_Set_Charmap(face, found);

	unsigned int ascent;
	unsigned int descent;
	unsigned int height;
	unsigned int lineskip;

	if (FT_IS_SCALABLE(face)) {
		err = FT_Set_Char_Size(face, 0, meta.fontSize * meta.fontDpi, 0, 0);
		if (err) {
			cerr << "Couldn't set font size\n";
			FT_Done_Face(face);
			FT_Done_FreeType(library);
			return nullptr;
		}

		const auto scale = face->size->metrics.y_scale;
		ascent = FT_CEIL(FT_MulFix(face->ascender, scale));
		descent = FT_CEIL(FT_MulFix(face->descender, scale));
		height = ascent - descent + /* baseline */ 1;
		lineskip = FT_CEIL(FT_MulFix(face->height, scale));
	} else {
		if (meta.fontSize >= (unsigned int)face->num_fixed_sizes)
			meta.fontSize = (unsigned int)face->num_fixed_sizes - 1;
		err = FT_Set_Pixel_Sizes(face,
			static_cast<FT_UInt>(face->available_sizes[meta.fontSize].width),
			static_cast<FT_UInt>(face->available_sizes[meta.fontSize].height));

		ascent = face->available_sizes[meta.fontSize].height;
		descent = 0;
		height = face->available_sizes[meta.fontSize].height;
		lineskip = FT_CEIL(ascent);
	}

	struct GlyphBitmap {
		uint8_t* buffer;
		unsigned int width;
		unsigned int height;
		unsigned int tx;
		unsigned int ty;
	};

	DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	unsigned int textureWidth = 1024;
	unsigned int textureHeight = 1024;
	unsigned int cx = 1;
	unsigned int cy = 1;
	unsigned int maxHeight = 0;

	jvector<FontKerning> kernings;
	jvector<FontGlyph> glyphs;
	jvector<FT_UInt> indices;
	jvector<GlyphBitmap> bitmaps;

	FT_UInt gindex;
	FT_ULong code = FT_Get_First_Char(face, &gindex);
	while (gindex != 0) {
		err = FT_Load_Glyph(face, gindex, FT_LOAD_DEFAULT);
		if (err) {
			cerr << L"Failed to load glyph!\n";
			FT_Done_Face(face);
			FT_Done_FreeType(library);
			return nullptr;
		}
		if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
			err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
			if (err) {
				cerr << L"Failed to render glyph!\n";
				FT_Done_Face(face);
				FT_Done_FreeType(library);
				return nullptr;
			}
		}

		auto metrics = face->glyph->metrics;

		GlyphBitmap bmp;
		auto src = face->glyph->bitmap;

		bmp.width = src.width;
		bmp.height = src.rows;
		bmp.buffer = new uint8_t[bmp.width * bmp.height];
		memcpy(bmp.buffer, src.buffer, bmp.width * bmp.height);

		bmp.tx = cx;
		bmp.ty = cy;
		cx += bmp.width + 1;
		maxHeight = max(maxHeight, bmp.height + 1);

		if (cy + maxHeight > textureHeight)
			textureHeight += 64;
		if (cx > textureWidth) {
			cx = 1;
			cy += maxHeight;
			maxHeight = 0;
		}

		FontGlyph g;
		g.character = (wchar_t)code;
		g.advance = FT_CEIL(metrics.horiAdvance);
		g.ox = FT_FLOOR(metrics.horiBearingX);
		g.oy = FT_FLOOR(metrics.horiBearingY);
		g.tx0 = bmp.tx;
		g.ty0 = bmp.ty;
		g.tw = bmp.width;
		g.th = bmp.height;

		indices.push_back(gindex);
		glyphs.push_back(g);
		bitmaps.push_back(bmp);

		code = FT_Get_Next_Char(face, code, &gindex);
	}

	if (FT_HAS_KERNING(face)) {
		for (int i = 0; i < glyphs.size(); i++) {
			for (int j = 0; j < glyphs.size(); j++) {
				if (i == j) continue;
				FT_Vector delta;
				err = FT_Get_Kerning(face, indices[i], indices[j], ft_kerning_default, &delta);
				if ((delta.x >> 6) > 0) {
					wchar_t from = glyphs[i].character;
					wchar_t to = glyphs[j].character;
					kernings.push_back({ from, to, delta.x >> 6 });
				}
			}
		}
	}

	uint8_t* texture = new uint8_t[textureWidth * textureHeight * 4];
	ZeroMemory(texture, textureWidth * textureHeight * 4);

	for (int i = 0; i < bitmaps.size(); i++) {
		for (unsigned int y = 0; y < bitmaps[i].height; y++) {
			for (unsigned int x = 0; x < bitmaps[i].width; x++) {
				unsigned int ti = ((x + bitmaps[i].tx) + (y + bitmaps[i].ty) * textureWidth) * 4;
				uint8_t c = bitmaps[i].buffer[x + y * bitmaps[i].width];
				texture[ti++] = 0xFF;
				texture[ti++] = 0xFF;
				texture[ti++] = 0xFF;
				texture[ti++] = c;
			}
		}
	}

	shared_ptr<Texture> tex = shared_ptr<Texture>(new Texture(
		L"Font Texture", textureWidth, textureHeight, 1,
		D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, textureFormat, ALPHA_MODE_TRANSPARENCY, 1,
		texture, textureWidth * textureHeight * 4, false));

	for (int i = 0; i < bitmaps.size(); i++)
		delete[] bitmaps[i].buffer;

	delete[] texture;

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return new Font(GetNameW(path), meta.fontSize, height, ascent, descent, lineskip, meta.fontDpi, tex, glyphs, kernings);
}