#include "AssetDatabase.hpp"
#include "../Common/Asset.hpp"
#include "../Common/AssetFile.hpp"
#include "../Common/TextureAsset.hpp"

#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

#include <d3d12.h>

using namespace std;

jmap<jstring, shared_ptr<Asset>> AssetDatabase::assets;

void AssetDatabase::LoadAssets(jstring file) {
	int count;
	AssetFile::AssetData* a = AssetFile::Read(file, count);
	for (int i = 0; i < count; i++) {
		Asset* t = nullptr;

		switch (a[i].type) {
		case AssetFile::TYPEID_MESH:
			t = new Mesh(a[i].name, *a[i].buffer);
			break;
		case AssetFile::TYPEID_SHADER:
			t = new Shader(a[i].name, *a[i].buffer);
			break;
		case AssetFile::TYPEID_TEXTURE:
			t = new Texture(a[i].name, *a[i].buffer);
			break;
		}

		if (t) {
			t->mGroup = file;
			assets.emplace(t->mName, shared_ptr<Asset>(t));
		}
	}
	delete[] a;
}
void AssetDatabase::UnloadAssets() {
	assets.clear();
}

shared_ptr<Asset> AssetDatabase::GetAsset(jstring name) {
	return assets.at(name);
}