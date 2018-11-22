#include "Asset.hpp"
#include "Util.hpp"

using namespace std;

Asset::Asset(string name) : m_Name(name) {}
Asset::~Asset() {}

void Asset::WriteHeader(ofstream &stream) {
	AssetHeader(stream, this);
}
void Asset::WriteData(ofstream &stream) {
	AssetData(stream, this);
}