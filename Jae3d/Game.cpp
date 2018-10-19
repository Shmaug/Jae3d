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

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

Camera* camera;
Mesh* mesh;

float yaw;
float pitch;

bool cursorVisible = true;

void Game::Initialize(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	camera = new Camera();
	camera->m_Position = { 0, .2f, 2, 0 };
	camera->m_Rotation = XMQuaternionIdentity();
	
	mesh = new Mesh();
	mesh->LoadObj("Assets/Models/icosphere.obj");
	mesh->Create();
}
Game::~Game() {
	delete mesh;
	delete camera;
}

void Game::OnResize() {
	camera->m_Aspect = (float)Graphics::m_ClientWidth / (float)Graphics::m_ClientHeight;
}

void Game::Update(double total, double delta) {
	if (Input::OnKeyDown(KeyCode::Enter) && Input::KeyDown(KeyCode::AltKey))
		Graphics::SetFullscreen(!Graphics::IsFullscreen());

	if (Input::OnKeyDown(KeyCode::AltKey))
		Input::m_LockMouse = !Input::m_LockMouse;

	if (Input::m_LockMouse) {
		XMINT2 md = Input::MouseDelta();
		yaw += md.x * .005f;
		pitch -= md.y * .005f;
		pitch = fmin(fmax(pitch, -XM_PIDIV2), XM_PIDIV2);
		camera->m_Rotation = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0);

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
	fwd = XMVector3Rotate(fwd, camera->m_Rotation);
	right = XMVector3Rotate(right, camera->m_Rotation);
	if (Input::KeyDown(KeyCode::W))
		camera->m_Position += fwd * (float)delta;
	if (Input::KeyDown(KeyCode::S))
		camera->m_Position -= fwd * (float)delta;
	if (Input::KeyDown(KeyCode::D))
		camera->m_Position += right * (float)delta;
	if (Input::KeyDown(KeyCode::A))
		camera->m_Position -= right * (float)delta;

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
	auto rtv = Graphics::GetCurrentRenderTargetView();
	auto dsv = Graphics::GetDepthStencilView();
	auto backBuffer = Graphics::GetBackBuffer();

	DirectX::XMFLOAT4 clearColor = { 0.4f, 0.6f, 0.9f, 1.f };
	commandList->ClearRenderTargetView(rtv, (float*)&clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	{
		D3D12_RESOURCE_BARRIER barriers[2] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(
				m_msaaRenderTarget.Get(), // TODO
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(
				backBuffer.Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RESOLVE_DEST)
		};

		commandList->ResourceBarrier(2, barriers);
	}

	commandList->ResolveSubresource(backBuffer, 0, m_msaaRenderTarget.Get(), 0, c_backBufferFormat);


	Graphics::SetCamera(commandList, camera);
	Graphics::SetShader(commandList, ShaderLibrary::GetShader("default"));
	Graphics::DrawMesh(commandList, mesh, XMMatrixIdentity());
}