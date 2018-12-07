#include "Button.hpp"

Button::Button(UDim2 pos, UDim2 size, jwstring text, std::function<void()> click) : UIControl(pos, size), mText(text), mClick(click) {}
Button::~Button() {}

void Button::Draw(HDC hdc, RECT &window, bool force) {
	if (!mDirty && !force) return;
	mDirty = false;

	RECT r = CalcRect(window);

	if (mClicked)
		FillRect(hdc, &r, Brushes::clickBrush);
	else if (mHovered)
		FillRect(hdc, &r, Brushes::hvrBrush);
	else
		FillRect(hdc, &r, Brushes::bgBrush);

	int pBg = SetBkMode(hdc, TRANSPARENT);
	HGDIOBJ pObj = SelectObject(hdc, Fonts::font11);
	int pTc = SetTextColor(hdc, RGB(241, 241, 241));

	TEXTMETRIC metric;
	GetTextMetrics(hdc, &metric);
	RECT textRect{ r.left, r.bottom - metric.tmHeight - 4, r.right, r.bottom };

	DrawText(hdc, mText.c_str(), (int)mText.length(), &textRect, DT_CENTER);

	SetTextColor(hdc, pTc);
	SelectObject(hdc, pObj);
	SetBkMode(hdc, pBg);
}
void Button::Update(WPARAM wParam, RECT &window, InputState input) {
	UIControl::Update(wParam, window, input);
	bool lmbdown = mHovered && mLastLMB && !input.lmb;
	mLastLMB = input.lmb;
	if (lmbdown)
		mClick();
}