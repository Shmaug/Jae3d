#pragma once

#include "../Jae3d/Util.hpp"
#include "Asset.hpp"

#include <dxgiformat.h>

class TextureAsset : public Asset {
public:
	static inline unsigned int ComputeNumMips(unsigned int Width, unsigned int Height) {
		unsigned int HighBit;
		_BitScanReverse((unsigned long*)&HighBit, Width | Height);
		return HighBit + 1;
	}

	TextureAsset(jstring name, int width, int height, int dimension, DXGI_FORMAT format, int mipLevels, void* dds, size_t ddsSize);
	TextureAsset(jstring name, MemoryStream &ms);
	~TextureAsset();

	void WriteData(MemoryStream &ms);
	uint64_t TypeId();

	unsigned int Width() const { return mWidth; }
	unsigned int Height() const { return mHeight; }
	unsigned int Dimension() const { return mDimension; }
	unsigned int MipLevels() const { return mMipLevels; }
	DXGI_FORMAT Format() const { return mFormat; }

private:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mDimension;
	unsigned int mMipLevels;
	DXGI_FORMAT mFormat;

protected:
	size_t mDDSDataSize;
	char* mDDSData;
};