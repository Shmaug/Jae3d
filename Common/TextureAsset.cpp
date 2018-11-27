#include "TextureAsset.hpp"
#include "AssetFile.hpp"
#include "IOUtil.hpp"

#include "MemoryStream.hpp"

using namespace std;

TextureAsset::TextureAsset(string name, int width, int height, int dimension, DXGI_FORMAT fmt, int mip) : Asset(name),
	m_Width(width), m_Height(height), m_Dimension(dimension), m_Format(fmt), m_MipLevels(mip) {

}
TextureAsset::TextureAsset(string name, MemoryStream &ms) : Asset(name) {
	m_Width = ms.Read<uint32_t>();
	m_Height = ms.Read<uint32_t>();
	m_Dimension = ms.Read<uint32_t>();
	m_MipLevels = ms.Read<uint32_t>();
	m_Format = (DXGI_FORMAT)ms.Read<uint32_t>();
	m_PixelSize = ms.Read<uint64_t>();
	m_Pixels = new uint8_t[m_PixelSize / sizeof(uint8_t)];
	ms.Read(reinterpret_cast<char*>(m_Pixels), m_PixelSize);
}
TextureAsset::~TextureAsset() {
	if (m_Pixels) delete[] m_Pixels;
}
uint64_t TextureAsset::TypeId() { return (uint64_t)AssetFile::TYPEID_TEXTURE; }

void TextureAsset::SetPixels(uint8_t *pixels, size_t size){
	if (m_Pixels && size != m_PixelSize)
		delete[] m_Pixels;
	m_Pixels = new uint8_t[size / sizeof(uint8_t)];
	m_PixelSize = size;
	memcpy(m_Pixels, pixels, size);
}

void TextureAsset::WriteData(MemoryStream &ms) {
	Asset::WriteData(ms);
	ms.Write((uint32_t)m_Width);
	ms.Write((uint32_t)m_Height);
	ms.Write((uint32_t)m_Dimension);
	ms.Write((uint32_t)m_MipLevels);
	ms.Write((uint32_t)m_Format);
	ms.Write((uint64_t)m_PixelSize);
	ms.Write(reinterpret_cast<const char*>(m_Pixels), m_PixelSize);
}