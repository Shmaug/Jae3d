#include "AssetDatabase.hpp"
#include "../Common/Asset.hpp"
#include "../Common/AssetFile.hpp"

#include "Mesh.hpp"
#include "Shader.hpp"

#include <d3d12.h>

using namespace std;

map<string, shared_ptr<Asset>> AssetDatabase::assets;

void AssetDatabase::LoadAssets(string file) {
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
			//assets[i] = new Texture(name, mems);
			break;
		}

		if (t) assets.emplace(t->m_Name, shared_ptr<Asset>(t));
	}
	delete[] a;
}
void AssetDatabase::UnloadAssets() {
	assets.clear();
}

shared_ptr<Asset> AssetDatabase::GetAsset(string name) {
	return assets[name];
}