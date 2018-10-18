#pragma once

#include "KeyCode.h"
#include <DirectXMath.h>

class Input {
public:
	static bool m_LockMouse;

	static void OnKeyDownEvent(KeyCode::Key key, bool state);
	static void OnMouseMoveEvent(int x, int y);
	static void OnMousePressEvent(int button, bool state);
	static void OnMouseWheelEvent(float delta);

	static bool KeyDown(KeyCode::Key key);
	static bool OnKeyDown(KeyCode::Key key);
	static bool ButtonDown(int button);
	static bool OnButtonDown(int button);
	static int MouseWheelDelta();
	static DirectX::XMINT2 MouseDelta();

	static void FrameEnd();
};