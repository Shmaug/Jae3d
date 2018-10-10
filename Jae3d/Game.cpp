#include "Game.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>

#include "Profiler.h";
#include "Input.h"

int frameCounter;
double elapsedSeconds;

using namespace DirectX;

void Game::Initialize(Graphics *graphics) {
	this->graphics = graphics;
}

void Game::Update(double total, double delta) {
	Profiler::FrameStart();

	// measure fps
	frameCounter++;
	elapsedSeconds += delta;
	if (elapsedSeconds > 1.0) {
		char buffer[128];
		sprintf_s(buffer, 128, "FPS: %.1f (%.1f)\n", frameCounter / elapsedSeconds, graphics->GetAndResetFPS() / elapsedSeconds);
		OutputDebugString(buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0;

		char pbuf[1024];
		Profiler::PrintLastFrame(pbuf, 1024);
		OutputDebugString(pbuf);
	}

	char ibuf[1024];
	int c = 0;
	for (int i = 0; i < 255; i++) {
		if (Input::KeyDown((KeyCode::Key)i)) {
			c += sprintf_s(ibuf + c, 1024 - c, "%s button down", KeyCode::key_str[i]);
		}
	}
	for (int i = 0; i < 5; i++) {
		if (Input::ButtonDown(i)) {
			c += sprintf_s(ibuf + c, 1024 - c, "%d button down", i);
		}
	}
	OutputDebugString(ibuf);

	Input::FrameEnd();
	Profiler::FrameEnd();
}