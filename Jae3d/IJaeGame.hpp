#pragma once

#include <memory>
#include <d3d12.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class CommandList;

class IJaeGame {
public:
	virtual void Initialize() = 0;
	virtual void WindowEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {};
	virtual void OnResize() = 0;
	virtual void DoFrame() = 0;
};