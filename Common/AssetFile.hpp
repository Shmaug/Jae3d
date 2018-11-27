#pragma once

#include <vector>
#include <fstream>

class Asset;
class MemoryStream;

class AssetFile {
public:
	enum TYPEID : uint64_t {
		TYPEID_UNSPECIFIED = 0,
		TYPEID_MESH = 1,
		TYPEID_SHADER = 2,
		TYPEID_TEXTURE = 3,
	};
	struct AssetData {
		std::string name;
		TYPEID type;
		MemoryStream* buffer;

		AssetData() : name(""), type(TYPEID_UNSPECIFIED), buffer(nullptr) {}
		AssetData(std::string n, TYPEID t, MemoryStream* buf) : name(n), type(t), buffer(buf) {}
		~AssetData();
	};

	static AssetData* Read(const std::string file, int &count);
	static void Write(const std::string file, std::vector<Asset*> &assets, uint64_t version = (uint64_t)0001);

private:
	static AssetData* AssetFile::Read_V1(std::istream &fs, int &count);
	static void AssetFile::Write_V1(std::ostream &os, std::vector<Asset*> &assets);
};

