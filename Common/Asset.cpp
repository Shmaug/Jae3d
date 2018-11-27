#include "Asset.hpp"
#include "AssetFile.hpp"

#include "MemoryStream.hpp"

using namespace std;

Asset::Asset(string name) : m_Name(name) {}
Asset::Asset(string name, MemoryStream &ms) : Asset(name) {}
Asset::~Asset() {}
uint64_t Asset::TypeId() { return (uint64_t)AssetFile::TYPEID_UNSPECIFIED; }

void Asset::WriteData(MemoryStream &ms) {}