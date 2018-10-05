#pragma once

#include "Graphics.h"
#include "Input.h"

class Game {
private:
	Graphics *graphics;
public:
	void KeyEvent(KeyEventArgs& e);
	void MouseEvent(MouseButtonEventArgs& e);
	void MouseMove(MouseMoveEventArgs& e);
	void MouseWheelEvent(int delta);

	void Initialize(Graphics *graphics);
	void Update(double totalTime, double deltaTime);
};

