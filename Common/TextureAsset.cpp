#include "TextureAsset.hpp"
#include "AssetFile.hpp"
#include "IOUtil.hpp"

#include "MemoryStream.hpp"

using namespace std;

TextureAsset::TextureAsset(jstring name, int width, int height, int dimension, DXGI_FORMAT fmt, int mip, void* dds, size_t ddsSize) : Asset(name),
	mWidth(width), mHeight(height), mDimension(dimension), mFormat(fmt), mMipLevels(mip), mDDSDataSize(ddsSize) {
	mDDSData = new char[ddsSize];
	memcpy(mDDSData, dds, ddsSize);
}
TextureAsset::TextureAsset(jstring name, MemoryStream &ms) : Asset(name) {
	mWidth = ms.Read<uint32_t>();
	mHeight = ms.Read<uint32_t>();
	mDimension = ms.Read<uint32_t>();
	mMipLevels = ms.Read<uint32_t>();
	mFormat = (DXGI_FORMAT)ms.Read<uint32_t>();
	mDDSDataSize = ms.Read<uint64_t>();
	mDDSData = new char[mDDSDataSize];
	ms.Read(reinterpret_cast<char*>(mDDSData), mDDSDataSize);
}
TextureAsset::~TextureAsset() {
	if (mDDSData) delete[] mDDSData;
}

uint64_t TextureAsset::TypeId() { return (uint64_t)AssetFile::TYPEID_TEXTURE; }

void TextureAsset::WriteData(MemoryStream &ms) {
	Asset::WriteData(ms);
	ms.Write((uint32_t)mWidth);
	ms.Write((uint32_t)mHeight);
	ms.Write((uint32_t)mDimension);
	ms.Write((uint32_t)mMipLevels);
	ms.Write((uint32_t)mFormat);
	ms.Write((uint64_t)mDDSDataSize);
	ms.Write(reinterpret_cast<const char*>(mDDSData), mDDSDataSize);
}