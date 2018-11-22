#pragma once
#include <string>
#include <dxgi.h>
#include "Asset.hpp"

class TextureAsset : public Asset {
public:
	TextureAsset(std::string name, int width, int height, int dimension, DXGI_FORMAT format, int mipLevels);
	~TextureAsset();

	void WriteHeader(std::ofstream &stream);
	void WriteData(std::ofstream &stream);

	void SetPixels(uint8_t *pixels, size_t size);

private:
	int m_Width;
	int m_Height;
	int m_Dimension;
	int m_MipLevels;
	DXGI_FORMAT m_Format;

	uint8_t *m_Pixels;
	size_t m_PixelSize;
};