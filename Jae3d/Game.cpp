#include "Game.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <locale.h>

#include "Profiler.hpp"
#include "Input.hpp"
#include "Util.hpp"
#include "Graphics.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Window.hpp"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

shared_ptr<Camera> camera;
shared_ptr<Mesh> barrel;
shared_ptr<Mesh> cube;
shared_ptr<Mesh> rifle;

float yaw;
float pitch;

bool cursorVisible = true;

void Game::Initialize(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	camera = shared_ptr<Camera>(new Camera("Camera"));
	camera->CreateCB();
	camera->LocalPosition({ 0, .4f, 1, 0 });
	
	barrel = shared_ptr<Mesh>(new Mesh("Barrel"));
	barrel->LoadFbx("Assets/Models/barrel.fbx");
	barrel->Create();
	barrel->LocalPosition({ 0, .2f, 0, 0 });
	
	cube = shared_ptr<Mesh>(new Mesh("Cube"));
	cube->LoadCube(1.0f);
	cube->Create();
	cube->LocalPosition({ 0, -.1f, 0, 0 });
	cube->LocalScale({10.0f, .1f, 10.0f, 1.0f});
	
	rifle = shared_ptr<Mesh>(new Mesh("Rifle"));
	rifle->LoadObj("Assets/Models/rifle.obj");
	rifle->Create();
	rifle->Parent(camera);
	rifle->LocalPosition({ -.25f, -.6f, -.25f, 0 });

	auto window = Graphics::GetWindow();
	camera->Aspect((float)window->GetWidth() / (float)window->GetHeight());
}
Game::~Game() {}

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
		yaw += md.x * .0025f;
		pitch -= md.y * .0025f;
		pitch = fmin(fmax(pitch, -XM_PIDIV2), XM_PIDIV2);
		camera->LocalRotation(XMQuaternionRotationRollPitchYaw(pitch, yaw, 0));

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
	fwd = XMVector3Rotate(fwd, camera->LocalRotation());
	right = XMVector3Rotate(right, camera->LocalRotation());
	XMVECTOR p = camera->LocalPosition();
	if (Input::KeyDown(KeyCode::W))
		p += fwd * (float)delta;
	if (Input::KeyDown(KeyCode::S))
		p -= fwd * (float)delta;
	if (Input::KeyDown(KeyCode::D))
		p += right * (float)delta;
	if (Input::KeyDown(KeyCode::A))
		p -= right * (float)delta;
	camera->LocalPosition(p);
#pragma endregion

	// fps stats
	static double timer = 0;
	timer += delta;
	if (timer >= 1.0) {

		char fps[16];
		PrintFormattedf(fps, sizeof(fps), "%.1f", (float)m_fps);

		timer = 0.0;
		char buffer[128];
		sprintf_s(buffer, 128, "FPS: %s\n", fps);
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
	barrel->Draw(commandList);
	cube->Draw(commandList);
	rifle->Draw(commandList);
}