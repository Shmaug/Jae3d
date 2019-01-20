#pragma once

#include "Common.hpp"
#include "AssetFile.hpp"
#include "Asset.hpp"
#include <unordered_map>

class AssetDatabase {
private:
	JAE_API static std::unordered_map<jwstring, std::shared_ptr<Asset>> assets;

public:
	class LoadAssetOperation {
	public:
		DWORD mThreadId;
		HANDLE mThreadHandle;
		jwstring mFile;
		float mProgress;
		bool mDone;
		jvector<std::shared_ptr<Asset>> mLoadedAssets;
	};

	JAE_API static LoadAssetOperation* LoadAssetsAsync(jwstring assetFile);
	JAE_API static void FinishLoadAssetsAsync(LoadAssetOperation* operation);
	JAE_API static void LoadAssets(jwstring assetFile);
	JAE_API static void UnloadAssets();
	JAE_API static std::shared_ptr<Asset> GetAsset(jwstring name);

	template<class T>
	static std::shared_ptr<T> GetAsset(jwstring name) {
		static_assert(std::is_base_of<Asset, T>::value, "T must be an Asset!");
		return std::static_pointer_cast<T>(assets.at(name.lower()));
	}
};