#pragma once

#include "UIControl.hpp"
#include <jstring.hpp>

class Label : public UIControl {
public:
	Label(UDim2 pos, UDim2 size, jwstring text, HFONT font);
	~Label();

	void Draw(HDC hdc, RECT &window, bool force = false);
	void Update(WPARAM wParam, RECT &window, InputState input);

private:
	HFONT mFont;
	jwstring mText;
};


