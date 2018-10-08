#include "Game.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>

#include "Mathf.h";
#include "Profiler.h";

int frameCounter;
double elapsedSeconds;

using namespace DirectX;

void Game::Initialize(Graphics *graphics) {
	this->graphics = graphics;
}

void Game::KeyEvent(KeyEventArgs& e) {

}
void Game::MouseEvent(MouseButtonEventArgs& e) {

}
void Game::MouseMove(MouseMoveEventArgs& e) {

}
void Game::MouseWheelEvent(int delta) {

}

void Game::Update(double total, double delta) {
	Profiler::FrameStart();

	// measure fps
	frameCounter++;
	elapsedSeconds += delta;
	if (elapsedSeconds > 1.0) {
		char buffer[128];
		sprintf_s(buffer, 128, "FPS: %.1f (%.1f)\n", frameCounter / elapsedSeconds, graphics->fpsCounter / elapsedSeconds);
		OutputDebugString(buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0;
		graphics->fpsCounter = 0;

		char pbuf[1024];
		Profiler::PrintLastFrame(pbuf, 1024);
		OutputDebugString(pbuf);
	}

	Profiler::FrameEnd();
}