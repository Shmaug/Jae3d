#pragma once

#include "UIControl.hpp"
#include <jstring.hpp>

#include <functional>

class Button : public UIControl {
public:
	Button(UDim2 pos, UDim2 size, jwstring text, std::function<void()> click);
	~Button();

	void Draw(HDC hdc, RECT &window, bool force = false);
	void Update(WPARAM wParam, RECT &window, InputState input);

private:
	jwstring mText;
	bool mLastLMB;
	std::function<void()> mClick;
};

