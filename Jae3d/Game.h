#pragma once

#include "Graphics.h"

class Game {
private:
	Graphics *graphics;
public:
	float m_fps;
	void Initialize(Graphics *graphics);
	void Update(double totalTime, double deltaTime);
};