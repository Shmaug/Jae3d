#pragma once

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

struct InputState {
	POINT cursor;
	bool lmb;
	bool rmb;
	bool lmbFirst;
	bool rmbFirst;
	int clickCount;
};

struct UDim2 {
	double xOffset;
	double yOffset;
	double xScale;
	double yScale;
};

class Fonts {
public:
	static HFONT font9;
	static HFONT font9Bold;
	static HFONT font11;
	static HFONT font11Bold;
	static HFONT font18;
	static HFONT font18Bold;
	static void Create();
};
class Brushes {
public:
	static HBRUSH bgDarkBrush;
	static HBRUSH bgBrush;
	static HBRUSH hvrBrush;
	static HBRUSH clickBrush;
	static HBRUSH selectedBrush;
	static void Create();
};

class UIControl {
public:
	bool mVisible;

	UIControl() {};
	UIControl(UDim2 pos, UDim2 size) : mPos(pos), mSize(size), mVisible(true) {};

	virtual void Draw(HDC hdc, RECT &window, bool force = false) = 0;
	virtual void Update(WPARAM wParam, RECT &window, InputState input);

	RECT CalcRect(RECT &window);

protected:
	UDim2 mPos;
	UDim2 mSize;

	bool mHovered;
	bool mClicked;
	bool mDirty = true;
};

