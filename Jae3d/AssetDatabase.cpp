#include "AssetDatabase.hpp"
#include "Asset.hpp"

#include <d3d12.h>

using namespace std;

std::map<std::string, std::shared_ptr<Asset>> AssetDatabase::assets;

void ParseToken(string token) {

}

void AssetDatabase::LoadAssets(string file) {
	// TODO load assets, assign group

}
void AssetDatabase::UnloadAssets(string group) {
	// TODO unload assets from group

}

shared_ptr<Asset> AssetDatabase::GetAsset(string name) {
	return assets[name];
}