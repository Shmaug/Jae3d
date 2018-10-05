#pragma once

#include "Graphics.h"

class Game {
public:
	Graphics *graphics;
	void Update(double totalTime, double deltaTime);
};

