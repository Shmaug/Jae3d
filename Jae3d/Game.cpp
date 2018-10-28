#include "Game.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <locale.h>

#include "Profiler.h"
#include "Input.h"
#include "Util.h"
#include "Graphics.h"
#include "Mesh.h"
#include "Camera.h"
#include "Shader.h"
#include "Window.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

Camera* camera;
Mesh* mesh;

float yaw;
float pitch;

bool cursorVisible = true;

void Game::Initialize(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	camera = new Camera("Camera");
	camera->Position({ 0, .4f, 1, 0 });
	
	mesh = new Mesh("Mesh");
	mesh->LoadObj("Assets/Models/sphere.obj");
	mesh->Create();

	auto window = Graphics::GetWindow();
	camera->Aspect((float)window->GetWidth() / (float)window->GetHeight());
}
Game::~Game() {
	delete mesh;
	delete camera;
}

void Game::OnResize() {
	auto window = Graphics::GetWindow();
	camera->Aspect((float)window->GetWidth() / (float)window->GetHeight());
}

void Game::Update(double total, double delta) {
	auto window = Graphics::GetWindow();
	if (Input::OnKeyDown(KeyCode::Enter) && Input::KeyDown(KeyCode::AltKey))
		window->SetFullscreen(!window->IsFullscreen());
	if (Input::OnKeyDown(KeyCode::F4) && Input::KeyDown(KeyCode::AltKey))
		window->Close();

#pragma region camera controls
	if (Input::OnKeyDown(KeyCode::AltKey))
		Input::m_LockMouse = !Input::m_LockMouse;

	if (Input::m_LockMouse) {
		XMINT2 md = Input::MouseDelta();
		yaw += md.x * .005f;
		pitch -= md.y * .005f;
		pitch = fmin(fmax(pitch, -XM_PIDIV2), XM_PIDIV2);
		camera->Rotation(XMQuaternionRotationRollPitchYaw(pitch, yaw, 0));

		if (cursorVisible) {
			ShowCursor(false);
			cursorVisible = false;
		}
	} else {
		if (!cursorVisible) {
			ShowCursor(true);
			cursorVisible = true;
		}
	}
	
	XMVECTOR fwd = { 0, 0, -1, 0 };
	XMVECTOR right = { -1, 0, 0, 0 };
	fwd = XMVector3Rotate(fwd, camera->Rotation());
	right = XMVector3Rotate(right, camera->Rotation());
	XMVECTOR p = camera->Position();
	if (Input::KeyDown(KeyCode::W))
		p += fwd * (float)delta;
	if (Input::KeyDown(KeyCode::S))
		p -= fwd * (float)delta;
	if (Input::KeyDown(KeyCode::D))
		p += right * (float)delta;
	if (Input::KeyDown(KeyCode::A))
		p -= right * (float)delta;
	camera->Position(p);
#pragma endregion

	// fps stats
	static double timer = 0;
	timer += delta;
	if (timer >= 1.0) {
		char fps[16];
		PrintFormattedf(fps, sizeof(fps), "%.1f", (float)m_fps);

		timer = 0.0;
		char buffer[128];
		sprintf_s(buffer, 128, "FPS: %s (%.1f)\n", fps, Graphics::m_fps);
		OutputDebugString(buffer);

		char pbuf[1024];
		Profiler::PrintLastFrame(pbuf, 1024);
		OutputDebugString(pbuf);
	}
}

void Game::Render(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	auto window = Graphics::GetWindow();
	auto rtv = window->GetCurrentRenderTargetView();
	auto dsv = window->GetDepthStencilView();

	DirectX::XMFLOAT4 clearColor = { 0.4f, 0.6f, 0.9f, 1.f };
	commandList->ClearRenderTargetView(rtv, (float*)&clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	Graphics::SetShader(commandList, ShaderLibrary::GetShader("default"));
	Graphics::SetCamera(commandList, camera);
	Graphics::DrawMesh(commandList, mesh);
}