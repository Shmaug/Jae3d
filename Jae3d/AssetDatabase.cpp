#include "AssetDatabase.hpp"

std::unordered_map<jwstring, std::shared_ptr<Asset>> AssetDatabase::assets;

void AssetDatabase::LoadAssets(jwstring assetFile) {
	jvector<Asset*> a = AssetFile::Read(assetFile);
	for (int i = 0; i < a.size(); i++) {
		if (a[i]) {
			a[i]->mGroup = assetFile;
			assets.emplace(a[i]->mName.lower(), std::shared_ptr<Asset>(a[i]));
		}
	}
}
void AssetDatabase::UnloadAssets() {
	assets.clear();
}

std::shared_ptr<Asset> AssetDatabase::GetAsset(jwstring name) {
	return assets.at(name.lower());
}