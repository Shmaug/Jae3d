#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Asset.hpp>
#include <memory>

class Viewport {
public:
	Viewport();
	~Viewport();

	void Init(HWND hwnd);
	void Resize();
	void DoFrame();

	void Show(std::shared_ptr<Asset> asset);
};

