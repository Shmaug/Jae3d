#pragma once

#include "Util.hpp"
#include "jstring.hpp"
#include "jvector.hpp"

#include <fstream>

class Asset;
class MemoryStream;

class AssetFile {
public:
	JAE_API static jvector<Asset*> Read(jwstring file);
	JAE_API static void Write(jwstring file, jvector<Asset*> assets, bool compress, uint64_t version = (uint64_t)0001);

private:
	JAE_API static jvector<Asset*> AssetFile::Read_V1(std::istream &fs);
	JAE_API static void AssetFile::Write_V1(std::ostream &os, jvector<Asset*> assets, bool compress);
};

