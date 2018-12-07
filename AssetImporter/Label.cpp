#include "Label.hpp"

Label::Label(UDim2 pos, UDim2 size, jwstring text, HFONT font) : UIControl(pos, size), mText(text), mFont(font) {}
Label::~Label() {}

void Label::Draw(HDC hdc, RECT &window, bool force) {
	if (!mDirty && !force) return;
	mDirty = false;

	RECT r = CalcRect(window);

	int pBg = SetBkMode(hdc, TRANSPARENT);
	HGDIOBJ pObj = SelectObject(hdc, mFont);
	int pTc = SetTextColor(hdc, RGB(241, 241, 241));

	TEXTMETRIC metric;
	GetTextMetrics(hdc, &metric);
	RECT textRect{ r.left, r.bottom - metric.tmHeight - 4, r.right, r.bottom };

	DrawText(hdc, mText.c_str(), (int)mText.length(), &textRect, DT_CENTER);

	SetTextColor(hdc, pTc);
	SelectObject(hdc, pObj);
	SetBkMode(hdc, pBg);
}
void Label::Update(WPARAM wParam, RECT &window, InputState input) {}