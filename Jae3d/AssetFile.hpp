#pragma once

#include "Util.hpp"
#include "jstring.hpp"
#include "jvector.hpp"

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
		TYPEID_FONT = 4,
	};

	JAE_API static Asset** Read(jwstring file, int &count);
	JAE_API static void Write(jwstring file, Asset** assets, size_t count, bool compress, uint64_t version = (uint64_t)0001);

private:
	JAE_API static Asset** AssetFile::Read_V1(std::istream &fs, int &count);
	JAE_API static void AssetFile::Write_V1(std::ostream &os, Asset** assets, size_t count, bool compress);
};

