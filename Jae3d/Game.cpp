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
#include "MeshRenderer.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Window.hpp"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

shared_ptr<Camera> camera;
shared_ptr<MeshRenderer> barrel;
shared_ptr<MeshRenderer> cube;
shared_ptr<MeshRenderer> rifle;

float yaw;
float pitch;

bool cursorVisible = true;

void Game::Initialize(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	camera = shared_ptr<Camera>(new Camera("Camera"));
	camera->CreateCB();
	camera->LocalPosition(0, 1.668f, -2.0f);
	camera->FieldOfView(60);

	shared_ptr<Mesh> barrelMesh = shared_ptr<Mesh>(new Mesh("Barrel"));
	barrelMesh->LoadFbx("Assets/Models/barrel.fbx", 2.75f);
	barrelMesh->Create();

	shared_ptr<Mesh> cubeMesh = shared_ptr<Mesh>(new Mesh("Cube"));
	cubeMesh->LoadCube(1.0f);
	cubeMesh->Create();

	shared_ptr<Mesh> rifleMesh = shared_ptr<Mesh>(new Mesh("Rifle"));
	rifleMesh->LoadObj("Assets/Models/rifle.obj", .6f);
	rifleMesh->Create();
	
	barrel = shared_ptr<MeshRenderer>(new MeshRenderer("Barrel"));
	barrel->m_Mesh = barrelMesh;
	barrel->LocalPosition(0, .574f, 0);
	
	cube = shared_ptr<MeshRenderer>(new MeshRenderer("Cube"));
	cube->m_Mesh = cubeMesh;
	cube->LocalPosition(0, -.1f, 0);
	cube->LocalScale(10.0f, .1f, 10.0f);
	
	rifle = shared_ptr<MeshRenderer>(new MeshRenderer("Rifle"));
	rifle->m_Mesh = rifleMesh;
	rifle->Parent(camera);
	rifle->LocalScale(.6f, .6f, .6f);
	rifle->LocalPosition(.21f, -.3f, .2f);

	auto window = Graphics::GetWindow();
	camera->Aspect((float)window->GetWidth() / (float)window->GetHeight());
}
Game::~Game() {}

void Game::OnResize() {
	auto window = Graphics::GetWindow();
	camera->Aspect((float)window->GetWidth() / (float)window->GetHeight());
}

void Game::Update(double total, double delta) {
	// TODO: switched to RH coordinate system - everything is backwards
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

	XMVECTOR fwd = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), camera->LocalRotation());
	XMVECTOR right = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), camera->LocalRotation());
	XMVECTOR d;
	if (Input::KeyDown(KeyCode::W))
		d += fwd * (float)delta;
	if (Input::KeyDown(KeyCode::S))
		d -= fwd * (float)delta;
	if (Input::KeyDown(KeyCode::D))
		d += right * (float)delta;
	if (Input::KeyDown(KeyCode::A))
		d -= right * (float)delta;
	if (!XMVector3Equal(d, XMVectorZero()))
		camera->LocalPosition(camera->LocalPosition() + XMVector3Normalize(d) * 1.4f);
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