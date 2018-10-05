#include "Game.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>

#include "Mathf.h";

int frameCounter;
int renderFrameCounter;
double elapsedSeconds;

using namespace DirectX;

XMFLOAT4 color;

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
	// measure fps
	frameCounter++;
	elapsedSeconds += delta;
	if (elapsedSeconds > 1.0) {
		char buffer[500];
		sprintf_s(buffer, 500, "FPS: %.1f (%.1f)\n", frameCounter / elapsedSeconds, renderFrameCounter / elapsedSeconds);
		OutputDebugString(buffer);

		frameCounter = 0;
		renderFrameCounter = 0;
		elapsedSeconds = 0.0;
	}

	// Render
	if (graphics->NextFrameReady()) { // Wait for GPU to finish rendering last frame
		renderFrameCounter++;

		auto commandAllocator = graphics->g_CommandAllocators[graphics->g_CurrentBackBufferIndex];
		auto backBuffer = graphics->g_BackBuffers[graphics->g_CurrentBackBufferIndex];

		commandAllocator->Reset();
		graphics->g_CommandList->Reset(commandAllocator.Get(), nullptr);

		graphics->ClearBackBuffer(backBuffer, color);
		graphics->Present(backBuffer);
	}
}