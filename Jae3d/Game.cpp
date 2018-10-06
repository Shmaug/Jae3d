#include "Game.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>

#include "Mathf.h";
#include "Profiler.h";

int frameCounter;
int renderFrameCounter;
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
		sprintf_s(buffer, 128, "FPS: %.1f (%.1f)\n", frameCounter / elapsedSeconds, renderFrameCounter / elapsedSeconds);
		OutputDebugString(buffer);

		frameCounter = 0;
		renderFrameCounter = 0;
		elapsedSeconds = 0.0;

		char pbuf[1024];
		Profiler::Print(pbuf, 1024);
		OutputDebugString(pbuf);
	}

	// Render
	if (graphics->NextFrameReady()) { // Wait for GPU to finish rendering last frame
		renderFrameCounter++;

		auto commandAllocator = graphics->g_CommandAllocators[graphics->g_CurrentBackBufferIndex];
		auto backBuffer = graphics->g_BackBuffers[graphics->g_CurrentBackBufferIndex];

		commandAllocator->Reset();
		graphics->g_CommandList->Reset(commandAllocator.Get(), nullptr);

		Profiler::BeginSample("Clear");
		graphics->ClearBackBuffer(backBuffer, XMFLOAT4(0.f, 0.f, 0.f, 1.f));
		Profiler::EndSample();

		Profiler::BeginSample("Present");
		graphics->Present(backBuffer);
		Profiler::EndSample();
	}

	Profiler::FrameEnd();
}