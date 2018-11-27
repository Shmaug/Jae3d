#include "AssetFile.hpp"

#include "IOUtil.hpp"

#include "Asset.hpp"
#include "MeshAsset.hpp"
#include "ShaderAsset.hpp"
#include "TextureAsset.hpp"
#include "AssetImporter.hpp"

#include "MemoryStream.hpp"

using namespace std;

Asset** AssetFile::Read_V1(istream &is, int &count) {
	uint32_t assetCount = ReadStream<uint32_t>(is);
	Asset** assets = new Asset*[assetCount];

	MemoryStream cmem(1024);
	MemoryStream mems(1024);

	for (uint32_t i = 0; i < assetCount; i++) {
		uint8_t id = ReadStream<uint8_t>(is);
		uint64_t type = ReadStream<uint64_t>(is);
		string name = ReadStream<string>(is);
		uint64_t size = ReadStream<uint64_t>(is);
		uint64_t sizeUncompressed = ReadStream<uint64_t>(is);

		if (AssetImporter::verbose)
			printf("%s (%lu bytes/%lu uncompressed)\n", name.c_str(), (unsigned long)size, (unsigned long)sizeUncompressed);

		uint64_t p = (int64_t)is.tellg();

		cmem.Seek(0);
		mems.Seek(0);
		cmem.Fit(size);
		mems.Fit(sizeUncompressed);

		is.read(cmem.Ptr(), size);

		cmem.Decompress(mems, size);
		mems.Seek(0);

		switch (type) {
		case TYPEID_MESH:
			assets[i] = new MeshAsset(name, mems);
			break;
		case TYPEID_SHADER:
			assets[i] = new ShaderAsset(name, mems);
			break;
		case TYPEID_TEXTURE:
			assets[i] = new TextureAsset(name, mems);
			break;
		}

		is.seekg(p + size);
	}

	count = assetCount;
	return assets;
}
void AssetFile::Write_V1(ostream &os, vector<Asset*> &assets) {
	WriteStream(os, (uint32_t)assets.size());

	MemoryStream mems(1024);
	MemoryStream cmem(1024);

	for (int i = 0; i < assets.size(); i++) {
		WriteStream(os, (uint8_t)0x01); // "item" identifier
		WriteStream(os, assets[i]->TypeId());
		WriteStream(os, assets[i]->m_Name);

		mems.Seek(0);
		assets[i]->WriteData(mems);
		uint64_t sizeUncompressed = (uint64_t)mems.Tell();

		cmem.Seek(0);
		mems.Compress(cmem);
		uint64_t size = (uint64_t)cmem.Tell();

		WriteStream(os, size);
		WriteStream(os, sizeUncompressed);
		os.write(cmem.Ptr(), size);

		if (AssetImporter::verbose)
			printf("%s (%lu bytes/%lu uncompressed)\n", assets[i]->m_Name.c_str(), (unsigned long)size, (unsigned long)sizeUncompressed);
	}
}

Asset** AssetFile::Read(const string file, int &count) {
	count = 0;
	Asset** assets = nullptr;

	ifstream is;
	is.open(file, ios::in | ios::binary);
	if (!is.is_open()) {
		perror("Failed to open file for reading!");
		return assets;
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
		assets = Read_V1(is, count);
		break;
	}

	return assets;
}
void AssetFile::Write(const string file, vector<Asset*> &assets, uint64_t version) {
	ofstream os;
	os.open(file, ios::out | ios::binary);
	if (!os.is_open()) {
		perror("Failed to open file for writing!");
		return;
	}

	printf("Writing %d assets to %s\n", (int)assets.size(), file.c_str());

	WriteStream(os, (uint64_t)14242); // 'magic' number
	os.write("ASSET", 5);
	WriteStream(os, version);

	switch (version) {
	default:
	case (uint64_t)0001:
		Write_V1(os, assets);
		break;
	}
}