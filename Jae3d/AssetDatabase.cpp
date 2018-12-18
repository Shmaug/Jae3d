#include "AssetDatabase.hpp"
#include "Asset.hpp"
#include "AssetFile.hpp"

#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

#include <d3d12.h>

using namespace std;

jmap<jwstring, shared_ptr<Asset>> AssetDatabase::assets;

void AssetDatabase::LoadAssets(jwstring file) {
	jvector<Asset*> a = AssetFile::Read(file);
	for (int i = 0; i < a.size(); i++) {
		if (a[i]) {
			a[i]->mGroup = file;
			assets.emplace(a[i]->mName, shared_ptr<Asset>(a[i]));
		}
	}
}
void AssetDatabase::UnloadAssets() {
	assets.clear();
}

shared_ptr<Asset> AssetDatabase::GetAsset(jwstring name) {
	return assets.at(name);
}