#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class IJaeGame;

HWND JaeCreateWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR title, int width, int height);
void JaeMsgLoop(IJaeGame* game);
void JaeDestroy();