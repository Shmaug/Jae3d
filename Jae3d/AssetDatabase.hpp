#pragma once

#include "Common.hpp"

class AssetDatabase {
private:
	JAE_API static jmap<jwstring, std::shared_ptr<Asset>> assets;

public:
	JAE_API static void LoadAssets(jwstring assetFile);
	JAE_API static void UnloadAssets();

	JAE_API static std::shared_ptr<Asset> GetAsset(jwstring name);

	template<class T>
	static std::shared_ptr<T> GetAsset(jwstring name) {
		static_assert(std::is_base_of<Asset, T>::value, "T must be an Asset!");
		return std::static_pointer_cast<T>(assets.at(name));
	}
};

