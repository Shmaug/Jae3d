#pragma once

#include "Util.hpp"

#include "KeyCode.hpp"
#include <DirectXMath.h>

class Input {
public:
	JAE_API static bool mLockMouse;

	JAE_API static void OnKeyDownEvent(KeyCode::Key key, bool state);
	JAE_API static void OnMouseMoveEvent(int x, int y);
	JAE_API static void OnMousePressEvent(int button, bool state);
	JAE_API static void OnMouseWheelEvent(float delta);

	JAE_API static bool KeyDown(KeyCode::Key key);
	JAE_API static bool OnKeyDown(KeyCode::Key key);
	JAE_API static bool ButtonDown(int button);
	JAE_API static bool OnButtonDown(int button);
	JAE_API static float MouseWheelDelta();
	JAE_API static DirectX::XMINT2 MouseDelta();

	JAE_API static void FrameEnd();

private:
	friend class Window;
	JAE_API static void MousePos(DirectX::XMINT2 p);
};