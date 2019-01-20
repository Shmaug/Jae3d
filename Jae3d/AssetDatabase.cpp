#include "AssetDatabase.hpp"

std::unordered_map<jwstring, std::shared_ptr<Asset>> AssetDatabase::assets;

DWORD WINAPI LoadAssetFile(LPVOID param) {
	AssetDatabase::LoadAssetOperation* operation = (AssetDatabase::LoadAssetOperation*)param;

	jvector<Asset*> a = AssetFile::Read(operation->mFile);
	for (int i = 0; i < a.size(); i++) {
		if (a[i]) {
			a[i]->mGroup = operation->mFile;
			operation->mLoadedAssets.push_back(std::shared_ptr<Asset>(a[i]));
		}
	}
	operation->mDone = true;

	return 0;
}
AssetDatabase::LoadAssetOperation* AssetDatabase::LoadAssetsAsync(jwstring assetFile) {
	AssetDatabase::LoadAssetOperation* operation = new AssetDatabase::LoadAssetOperation();
	operation->mDone = false;
	operation->mProgress = 0;
	operation->mFile = assetFile;
	operation->mThreadHandle = CreateThread(NULL, 0, LoadAssetFile, (LPVOID)operation, 0, &operation->mThreadId);
	return operation;
}
void AssetDatabase::FinishLoadAssetsAsync(AssetDatabase::LoadAssetOperation* operation) {
	for (unsigned int i = 0; i < operation->mLoadedAssets.size(); i++)
		assets.emplace(operation->mLoadedAssets[i]->mName.lower(), operation->mLoadedAssets[i]);
	delete operation;
}

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