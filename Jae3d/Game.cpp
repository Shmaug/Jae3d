#include "Game.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>

#include "Profiler.h";
#include "Input.h"

using namespace DirectX;

void Game::Initialize(Graphics *graphics) {
	this->graphics = graphics;
}

void Game::Update(double total, double delta) {
	if (Input::OnKeyDown(KeyCode::Enter) && Input::KeyDown(KeyCode::AltKey))
		graphics->SetFullscreen(!graphics->IsFullscreen());

	static double timer = 0;
	timer += delta;
	if (timer >= 1.0) {
		timer = 0.0;
		char buffer[128];
		sprintf_s(buffer, 128, "FPS: %.1f (%.1f)\n", m_fps, graphics->m_fps);
		OutputDebugString(buffer);

		char pbuf[1024];
		Profiler::PrintLastFrame(pbuf, 1024);
		OutputDebugString(pbuf);
	}
}