#pragma once

#include "../Common/jstring.hpp"
#include "../Common/jmap.hpp"

#include "Util.hpp"

#include <memory>

class Asset;

class AssetDatabase {
private:
	static jmap<jstring, std::shared_ptr<Asset>> assets;

public:
	static void LoadAssets(jstring assetFile);
	static void UnloadAssets();

	static std::shared_ptr<Asset> GetAsset(jstring name);

	template<class T>
	static std::shared_ptr<T> GetAsset(jstring name) {
		static_assert(std::is_base_of<Asset, T>::value, "T must be an Asset!");
		return std::static_pointer_cast<T>(assets.at(name));
	}
};

