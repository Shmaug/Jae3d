#pragma once

#include "Util.hpp"

class IJaeGame;

JAE_API HWND JaeCreateWindow(LPCWSTR title, int width, int height, unsigned int bufferCount);
JAE_API void JaeMsgLoop(IJaeGame* game);
JAE_API void JaeDestroy();