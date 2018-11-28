#pragma once
#include <string>
#include <dxgiformat.h>
#include "Asset.hpp"

class TextureAsset : public Asset {
public:
	TextureAsset(std::string name, int width, int height, int dimension, DXGI_FORMAT format, int mipLevels);
	TextureAsset(std::string name, MemoryStream &ms);
	~TextureAsset();

	void WriteData(MemoryStream &ms);
	uint64_t TypeId();

	uint8_t* GetPixels() const { return m_Pixels; }
	void SetPixels(uint8_t *pixels, size_t size);

	unsigned int Width() const { return m_Width; }
	unsigned int Height() const { return m_Height; }
	unsigned int Dimension() const { return m_Dimension; }
	unsigned int MipLevels() const { return m_MipLevels; }
	size_t PixelSize() const { return m_PixelSize; }
	DXGI_FORMAT Format() const { return m_Format; }

private:
	unsigned int m_Width;
	unsigned int m_Height;
	unsigned int m_Dimension;
	unsigned int m_MipLevels;
	DXGI_FORMAT m_Format;

	uint8_t *m_Pixels;
	size_t m_PixelSize;
};