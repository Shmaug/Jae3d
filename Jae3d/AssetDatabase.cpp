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
	int count;
	Asset** a = AssetFile::Read(file, count);
	for (int i = 0; i < count; i++) {
		if (a[i]) {
			a[i]->mGroup = file;
			assets.emplace(a[i]->mName, shared_ptr<Asset>(a[i]));
		}
	}
	delete[] a;
}
void AssetDatabase::UnloadAssets() {
	assets.clear();
}

shared_ptr<Asset> AssetDatabase::GetAsset(jwstring name) {
	return assets.at(name);
}