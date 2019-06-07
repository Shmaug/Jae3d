#pragma once

#include "Util.hpp"

class IJaeGame;

JAE_API HWND JaeCreateWindow(LPCWSTR title, int width, int height, unsigned int bufferCount, bool allowTearing,
	int nCategories = 0, D3D12_MESSAGE_CATEGORY* suppressCategories = nullptr, int nMessageIDs = 0, D3D12_MESSAGE_ID* suppressMessageIDs = nullptr);
JAE_API void JaeMsgLoop(IJaeGame* game);
JAE_API void JaeDestroy();