#include "Asset.hpp"
#include "AssetFile.hpp"

#include "MemoryStream.hpp"

using namespace std;

Asset::Asset(const jwstring& name) : mName(name) {}
Asset::Asset(const jwstring& name, MemoryStream &ms) : Asset(name) {}
Asset::~Asset() {}
uint64_t Asset::TypeId() { return (uint64_t)ASSET_TYPE_UNSPECIFIED; }