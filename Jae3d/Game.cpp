#include "Game.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>

#include "Mathf.h";

int frameCounter;
double elapsedSeconds;

void Game::Update(double total, double delta) {
	// measure fps
	frameCounter++;
	elapsedSeconds += delta;
	if (elapsedSeconds > 1.0) {
		char buffer[500];
		auto fps = frameCounter / elapsedSeconds;
		sprintf_s(buffer, 500, "FPS: %f\n", fps);
		OutputDebugString(buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0;
	}

	auto commandAllocator = graphics->g_CommandAllocators[graphics->g_CurrentBackBufferIndex];
	auto backBuffer = graphics->g_BackBuffers[graphics->g_CurrentBackBufferIndex];

	commandAllocator->Reset();
	graphics->g_CommandList->Reset(commandAllocator.Get(), nullptr);

	graphics->ClearBackBuffer(backBuffer, Color(cosf(total) * .5f + .5f, sinf(total) * .5f + .5f, 1.0));
	graphics->Present(backBuffer);
}