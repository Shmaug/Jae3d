#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Asset.hpp>
#include <memory>

#include "UIControl.hpp"
#include "ImportCommon.hpp"

class Properties : public UIControl {
public:
	Properties(UDim2 pos, UDim2 size, std::function<void()> reload);
	~Properties();

	void Draw(HDC hdc, RECT &window, bool force = false);
	void Update(WPARAM wParam, RECT &window, InputState input);

	void Show(AssetMetadata &asset);

private:
	std::function<void()> reload;
	AssetMetadata shownAsset;
};