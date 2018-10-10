#pragma once

#include "KeyCode.h"
#include <DirectXMath.h>

class Input {
public:
	static void Input::OnKeyDownEvent(KeyCode::Key key, bool state);
	static void Input::OnMouseMoveEvent(int x, int y);
	static void Input::OnMousePressEvent(int button, bool state);
	static void Input::OnMouseWheelEvent(int delta);

	static bool Input::KeyDown(KeyCode::Key key);
	static bool Input::OnKeyDown(KeyCode::Key key);
	static bool Input::ButtonDown(int button);
	static bool Input::OnButtonDown(int button);
	static int Input::MouseWheelDelta();
	static DirectX::XMINT2 Input::MouseDelta();

	static void Input::FrameEnd();
};