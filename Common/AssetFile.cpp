#include "AssetFile.hpp"

#include "../Jae3d/Util.hpp"
#include "IOUtil.hpp"

#include "Asset.hpp"
#include "MeshAsset.hpp"
#include "ShaderAsset.hpp"
#include "TextureAsset.hpp"

#include "MemoryStream.hpp"

using namespace std;

AssetFile::AssetData::~AssetData() {
	if (buffer) delete buffer;
}

AssetFile::AssetData* AssetFile::Read_V1(istream &is, int &count) {
	uint32_t assetCount = ReadStream<uint32_t>(is);

	AssetData* assets = new AssetData[assetCount];

	MemoryStream cmem(1024);

	for (uint32_t i = 0; i < assetCount; i++) {
		uint8_t id = ReadStream<uint8_t>(is);
		uint64_t type = ReadStream<uint64_t>(is);
		jstring name = ReadStream<jstring>(is);

		bool compressed = ReadStream<uint8_t>(is);
		uint64_t size = ReadStream<uint64_t>(is);
		uint64_t sizeUncompressed = ReadStream<uint64_t>(is);

		uint64_t p = (int64_t)is.tellg();

		assets[i].name = name;
		assets[i].type = (TYPEID)type;

		if (compressed) {
			cmem.Seek(0);
			cmem.Fit(size);
			is.read(cmem.Ptr(), size);

			MemoryStream* mems = new MemoryStream(sizeUncompressed, false);
			cmem.Decompress(*mems, size);
			mems->Seek(0);
			assets[i].buffer = mems;
		} else {
			MemoryStream* mems = new MemoryStream(sizeUncompressed, false);
			is.read(mems->Ptr(), sizeUncompressed);
			mems->Seek(0);
			assets[i].buffer = mems;
		}

		is.seekg(p + size);
	}

	count = assetCount;
	return assets;
}
void AssetFile::Write_V1(ostream &os, Asset** assets, size_t count, bool compress) {
	WriteStream(os, (uint32_t)count);

	MemoryStream mems(1024);
	MemoryStream cmem(1024);

	for (int i = 0; i < count; i++) {
		WriteStream(os, (uint8_t)0x01); // "item" identifier
		WriteStream(os, assets[i]->TypeId());
		WriteStream(os, assets[i]->mName);

		WriteStream(os, (uint8_t)compress);

		mems.Seek(0);
		assets[i]->WriteData(mems);
		uint64_t sizeUncompressed = (uint64_t)mems.Tell();
		if (compress) {
			cmem.Seek(0);
			mems.Compress(cmem);
			uint64_t size = (uint64_t)cmem.Tell();

			WriteStream(os, size);
			WriteStream(os, sizeUncompressed);
			os.write(cmem.Ptr(), size);
		} else {
			WriteStream(os, sizeUncompressed); // compressed size
			WriteStream(os, sizeUncompressed); // uncompressed size
			os.write(mems.Ptr(), sizeUncompressed);
		}
	}
}

AssetFile::AssetData* AssetFile::Read(jstring file, int &count) {
	count = 0;

	jstring fullpath = GetFullPath(file);

	ifstream is;
	is.open(string(fullpath.c_str()), ios::in | ios::binary);
	if (!is.is_open()) {
		OutputDebugf("Failed to open file %s for reading!\n", fullpath.c_str());
		cerr << "Failed to open file " << fullpath.c_str() << " for reading!\n";
		return nullptr;
	}

	// magic number
	if (ReadStream<uint64_t>(is) != (uint64_t)14242) return nullptr;

	// ASSET text
	char am[5];
	is.read(am, 5);
	if (am[0] != 'A' ||
		am[1] != 'S' ||
		am[2] != 'S' ||
		am[3] != 'E' ||
		am[4] != 'T') return nullptr;

	uint64_t version = ReadStream<uint64_t>(is);

	switch (version) {
	default:
	case (uint64_t)0001:
		return Read_V1(is, count);
	}

	return nullptr;
}
void AssetFile::Write(jstring file, Asset** assets, size_t count, bool compress, uint64_t version) {
	ofstream os;
	os.open(string(file.c_str()), ios::out | ios::binary);
	if (!os.is_open()) {
		cerr << "Failed to open file for writing!\n";
		return;
	}

	printf("Writing %d assets to %s\n", (int)count, file.c_str());

	WriteStream(os, (uint64_t)14242); // 'magic' number
	os.write("ASSET", 5);
	WriteStream(os, version);

	switch (version) {
	default:
	case (uint64_t)0001:
		Write_V1(os, assets, count, compress);
		break;
	}
}