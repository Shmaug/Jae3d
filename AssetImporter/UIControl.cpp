#include "UIControl.hpp"

HFONT Fonts::font9;
HFONT Fonts::font9Bold;
HFONT Fonts::font11;
HFONT Fonts::font11Bold;
HFONT Fonts::font18;
HFONT Fonts::font18Bold;
void Fonts::Create() {
	LOGFONT logFont;

	ZeroMemory(&logFont, sizeof(logFont));
	logFont.lfHeight = -9;
	logFont.lfWeight = FW_SEMIBOLD;
	wcscpy_s(logFont.lfFaceName, 32, L"Segoe UI");
	font9 = CreateFontIndirect(&logFont);

	ZeroMemory(&logFont, sizeof(logFont));
	logFont.lfHeight = -9;
	logFont.lfWeight = FW_BOLD;
	wcscpy_s(logFont.lfFaceName, 32, L"Segoe UI");
	font9Bold = CreateFontIndirect(&logFont);


	ZeroMemory(&logFont, sizeof(logFont));
	logFont.lfHeight = -11;
	logFont.lfWeight = FW_NORMAL;
	wcscpy_s(logFont.lfFaceName, 32, L"Segoe UI");
	font11 = CreateFontIndirect(&logFont);

	ZeroMemory(&logFont, sizeof(logFont));
	logFont.lfHeight = -11;
	logFont.lfWeight = FW_BOLD;
	wcscpy_s(logFont.lfFaceName, 32, L"Segoe UI");
	font11Bold = CreateFontIndirect(&logFont);

	ZeroMemory(&logFont, sizeof(logFont));
	logFont.lfHeight = -18;
	logFont.lfWeight = FW_NORMAL;
	wcscpy_s(logFont.lfFaceName, 32, L"Segoe UI");
	font18 = CreateFontIndirect(&logFont);

	ZeroMemory(&logFont, sizeof(logFont));
	logFont.lfHeight = -18;
	logFont.lfWeight = FW_BOLD;
	wcscpy_s(logFont.lfFaceName, 32, L"Segoe UI");
	font18Bold = CreateFontIndirect(&logFont);
}

HBRUSH Brushes::bgDarkBrush;
HBRUSH Brushes::bgBrush;
HBRUSH Brushes::hvrBrush;
HBRUSH Brushes::clickBrush;
HBRUSH Brushes::selectedBrush;
void Brushes::Create(){
	bgDarkBrush = CreateSolidBrush(RGB(37, 37, 38));
	bgBrush = CreateSolidBrush(RGB(45, 45, 48));
	hvrBrush = CreateSolidBrush(RGB(62, 62, 64));
	clickBrush = CreateSolidBrush(RGB(27, 27, 28));
	selectedBrush = CreateSolidBrush(RGB(0, 122, 204));
}

void UIControl::Update(WPARAM wParam, RECT &window, InputState input) {
	RECT r = CalcRect(window);

	bool hvr = mHovered;
	bool clicked = mClicked;
	mHovered = input.cursor.x > r.left && input.cursor.x < r.right && input.cursor.y < r.bottom && input.cursor.y > r.top;
	mClicked = mHovered && input.lmb;
	mDirty = mDirty || hvr != mHovered || clicked != mClicked;
}

RECT UIControl::CalcRect(RECT &window) {
	LONG w = window.right - window.left;
	LONG h = window.bottom - window.top;
	return {
		(LONG)(window.left + mPos.xOffset + mPos.xScale * w),
		(LONG)(window.top  + mPos.yOffset + mPos.yScale * h),
		(LONG)(mPos.xOffset + mPos.xScale * w + mSize.xOffset + mSize.xScale * w),
		(LONG)(mPos.yOffset + mPos.yScale * h + mSize.yOffset + mSize.yScale * h),
	};
}