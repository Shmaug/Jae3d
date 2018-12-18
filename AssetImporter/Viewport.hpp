#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Asset.hpp>
#include <memory>
#include "ImportCommon.hpp"

class Viewport {
public:
	void Init(HWND hwnd);
	void Resize();
	void DoFrame();

	void Show(AssetMetadata &asset);

private:
	AssetMetadata shownAsset;
};
