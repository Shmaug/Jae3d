#include "Game.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
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

void Game::Initialize(ComPtr<ID3D12GraphicsCommandList2> commandList) {
	camera = new Camera();
	camera->m_Position = { 0, 0, 3, 0 };
	
	mesh = new Mesh();
	mesh->LoadObj(L"Assets/Models/dragon.obj");
	mesh->Create(commandList);
}
Game::~Game() {
	delete(mesh);
	delete(camera);
}

void Game::OnResize() {
	camera->m_Aspect = (float)Graphics::m_ClientWidth / (float)Graphics::m_ClientHeight;
}

void Game::Update(double total, double delta) {
	if (Input::OnKeyDown(KeyCode::Enter) && Input::KeyDown(KeyCode::AltKey))
		Graphics::SetFullscreen(!Graphics::IsFullscreen());

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

	DirectX::XMFLOAT4 clearColor = { 0.4f, 0.6f, 0.9f, 1.f };
	commandList->ClearRenderTargetView(rtv, (float*)&clearColor, 0, nullptr);

	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	Graphics::SetCamera(commandList, camera);
	Graphics::SetShader(commandList, ShaderLibrary::GetShader("default"));
	Graphics::DrawMesh(commandList, mesh, XMMatrixIdentity());
}