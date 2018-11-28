#pragma once

#include <string>
#include <memory>
#include <unordered_map>

class Asset;

class AssetDatabase {
private:
	static std::unordered_map<std::string, std::shared_ptr<Asset>> assets;

public:
	static void LoadAssets(std::string assetFile);
	static void UnloadAssets();

	static std::shared_ptr<Asset> GetAsset(std::string name);

	template<class T>
	static std::shared_ptr<T> GetAsset(std::string name) {
		static_assert(std::is_base_of<Asset, T>::value, "T must be an Asset!");

		if (assets.count(name))
			return std::static_pointer_cast<T>(assets.at(name));
		else
			throw std::exception("Could not find asset!");

		return nullptr;
	}
};

