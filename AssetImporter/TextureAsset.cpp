#include "TextureAsset.hpp"

using namespace std;

TextureAsset::TextureAsset(string name, int width, int height, int dimension, DXGI_FORMAT fmt, int mip) : Asset(name),
	m_Width(width), m_Height(height), m_Dimension(dimension), m_Format(fmt), m_MipLevels(mip) {

}
TextureAsset::~TextureAsset() {
	if (m_Pixels)
		delete[] m_Pixels;
}

void TextureAsset::SetPixels(uint8_t *pixels, size_t size){
	if (m_Pixels && size != m_PixelSize)
		delete[] m_Pixels;
	m_Pixels = new uint8_t[size];
	memcpy(m_Pixels, pixels, size * sizeof(uint8_t));
}

void TextureAsset::WriteHeader(ofstream &stream) {
	Asset::WriteHeader(stream);

}
void TextureAsset::WriteData(ofstream &stream) {
	Asset::WriteData(stream);

}