#pragma once
#include "KeyCode.h"

class KeyEventArgs {
public:
	enum KeyState {
		Released = 0,
		Pressed = 1
	};

	KeyCode::Key    key;
	KeyState        state;

	KeyEventArgs(KeyCode::Key key, KeyState state)
		: key(key)
		, state(state) {}
};

class MouseMoveEventArgs {
public:
	int x;              // The X-position of the cursor relative to the upper-left corner of the client area.
	int y;              // The Y-position of the cursor relative to the upper-left corner of the client area.

	MouseMoveEventArgs(int x, int y)
		: x(x)
		, y(y) {}
};

class MouseButtonEventArgs {
public:
	enum ButtonState {
		Released = 0,
		Pressed = 1
	};

	int button;
	ButtonState state;
	MouseButtonEventArgs(int buttonID, ButtonState state)
		: button(buttonID)
		, state(state) {}
};
