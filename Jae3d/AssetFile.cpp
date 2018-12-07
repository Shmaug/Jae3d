#include "AssetFile.hpp"

#include "Util.hpp"
#include "IOUtil.hpp"

#include "Asset.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"

#include "MemoryStream.hpp"

using namespace std;

Asset** AssetFile::Read_V1(istream &is, int &count) {
	uint32_t assetCount = ReadStream<uint32_t>(is);

	Asset** assets = new Asset*[assetCount];

	MemoryStream mems(1024);
	MemoryStream cmem(1024);

	for (uint32_t i = 0; i < assetCount; i++) {
		uint8_t id = ReadStream<uint8_t>(is);
		uint64_t type = ReadStream<uint64_t>(is);
		jwstring name = ReadStream<jwstring>(is);
		OutputDebugString(name.c_str());
		OutputDebugString(L"\n");

		bool compressed = ReadStream<uint8_t>(is);
		uint64_t size = ReadStream<uint64_t>(is);
		uint64_t sizeUncompressed = ReadStream<uint64_t>(is);

		uint64_t p = (int64_t)is.tellg();

		mems.Fit(sizeUncompressed);
		mems.Seek(0);

		if (compressed) {
			cmem.Seek(0);
			cmem.Fit(size);
			is.read(cmem.Ptr(), size);

			cmem.Decompress(mems, size);
			mems.Seek(0);
		} else {
			is.read(mems.Ptr(), sizeUncompressed);
			mems.Seek(0);
		}

		switch (type) {
		case TYPEID_MESH:
			assets[i] = new Mesh(name, mems);
			break;
		case TYPEID_SHADER:
			assets[i] = new Shader(name, mems);
			break;
		case TYPEID_TEXTURE:
			assets[i] = new Texture(name, mems);
			break;
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
		wprintf(L"%s\n", assets[i]->mName.c_str());
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

Asset** AssetFile::Read(jwstring file, int &count) {
	count = 0;

	jwstring fullpath = GetFullPathW(file);

	ifstream is;
	is.open(fullpath.c_str(), ios::in | ios::binary);
	if (!is.is_open()) {
		OutputDebugf(L"Failed to open file %s for reading!\n", fullpath.c_str());
		cerr << "Failed to open file " << fullpath.c_str() << " for reading!\n";
		return nullptr;
	}

	// magic number
	if (ReadStream<uint64_t>(is) != (uint64_t)14242) {
		OutputDebugf(L"Wrong magic number!\n", fullpath.c_str());
		cerr << "Failed to open file " << fullpath.c_str() << " for reading!\n";
		return nullptr;
	}

	// ASSET text
	char am[5];
	is.read(am, 5);
	if (am[0] != 'A' ||
		am[1] != 'S' ||
		am[2] != 'S' ||
		am[3] != 'E' ||
		am[4] != 'T') {
		OutputDebugf(L"Wrong ASSET identifier!\n", fullpath.c_str());
		cerr << "Failed to open file " << fullpath.c_str() << " for reading!\n";
		return nullptr;
	}

	uint64_t version = ReadStream<uint64_t>(is);

	switch (version) {
	default:
	case (uint64_t)0001:
		return Read_V1(is, count);
	}

	return nullptr;
}
void AssetFile::Write(jwstring file, Asset** assets, size_t count, bool compress, uint64_t version) {
	ofstream os;
	os.open(file.c_str(), ios::out | ios::binary);
	if (!os.is_open()) {
		cerr << "Failed to open file for writing!\n";
		return;
	}

	printf("Writing %d assets to %S\n", (int)count, file.c_str());

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