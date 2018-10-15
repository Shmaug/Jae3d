#include <utility>
#include "Input.h"

bool state[260]; // 0-254 are keys, 255-259 are mouse buttons
bool lastState[260];
DirectX::XMINT2 mousePos;
DirectX::XMINT2 lastMousePos;
int wheelDelta;

void Input::OnKeyDownEvent(KeyCode::Key key, bool down) {
	state[key] = down;
}
void Input::OnMousePressEvent(int button, bool down) {
	state[259 - button] = down;
}
void Input::OnMouseMoveEvent(int x, int y) {
	mousePos = { x, y };
}
void Input::OnMouseWheelEvent(float delta) {
	wheelDelta += delta;
}

bool Input::KeyDown(KeyCode::Key key) {
	return state[key];
}
bool Input::OnKeyDown(KeyCode::Key key) {
	return state[key] && !lastState[key];
}
bool Input::ButtonDown(int button) {
	return state[259 - button];
}
bool Input::OnButtonDown(int button) {
	return state[259 - button] && !lastState[259 - button];
}
int Input::MouseWheelDelta() {
	return wheelDelta;
}
DirectX::XMINT2 Input::MouseDelta() {
	return { mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y };
}
void Input::FrameEnd() {
	memcpy(lastState, state, sizeof(state));
	lastMousePos = mousePos;
	wheelDelta = 0;
}