#include "Game.hpp"

#include "Profiler.hpp"
#include "Input.hpp"
#include "Util.hpp"
#include "Window.hpp"
#include "Graphics.hpp"
#include "CommandQueue.hpp"
#include "CommandList.hpp"
#include "AssetDatabase.hpp"

#include "MeshRenderer.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Material.hpp"
#include "Texture.hpp"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

shared_ptr<Camera> camera;
shared_ptr<MeshRenderer> cube;
shared_ptr<MeshRenderer> barrel;
shared_ptr<MeshRenderer> rifle;
shared_ptr<MeshRenderer> rifle2;
shared_ptr<MeshRenderer> dragon;

float yaw;
float pitch;

bool cursorVisible = true;

void Game::Initialize() {
	AssetDatabase::LoadAssets("models.asset");
	AssetDatabase::LoadAssets("shaders.asset");
	AssetDatabase::LoadAssets("textures.asset");

	camera = shared_ptr<Camera>(new Camera("Camera"));
	camera->CreateCB();
	camera->LocalPosition(0, 1.668f, -2.0f);
	camera->FieldOfView(60);
	auto window = Graphics::GetWindow();
	camera->Aspect((float)window->GetWidth() / (float)window->GetHeight());

	shared_ptr<Shader> shader = AssetDatabase::GetAsset<Shader>("default");
	shared_ptr<Shader> textured = AssetDatabase::GetAsset<Shader>("textured");

	shared_ptr<Mesh> cubeMesh = shared_ptr<Mesh>(new Mesh("Cube"));
	shared_ptr<Mesh> barrelMesh = AssetDatabase::GetAsset<Mesh>("Barrel");
	shared_ptr<Mesh> rifleMesh = AssetDatabase::GetAsset<Mesh>("Rifle.Rifle");
	shared_ptr<Mesh> dragonMesh = AssetDatabase::GetAsset<Mesh>("Dragon");

	cubeMesh->LoadCube(1.0f);

	cubeMesh->Upload();
	barrelMesh->Upload();
	rifleMesh->Upload();
	dragonMesh->Upload();

	shared_ptr<Material> defaultMaterial = shared_ptr<Material>(new Material("Default", shader));
	shared_ptr<Material> barrelMaterial = shared_ptr<Material>(new Material("Barrel", textured));
	shared_ptr<Material> rifleMaterial = shared_ptr<Material>(new Material("Rifle", textured));

	shared_ptr<Texture> barrelTexture = AssetDatabase::GetAsset<Texture>("barrel_albedo_roughness");
	shared_ptr<Texture> barrelNormalTexture = AssetDatabase::GetAsset<Texture>("barrel_normal");
	shared_ptr<Texture> barrelMetallicTexture = AssetDatabase::GetAsset<Texture>("barrel_metallic");
	shared_ptr<Texture> rifleTexture = AssetDatabase::GetAsset<Texture>("rifle_albedo_roughness");
	shared_ptr<Texture> rifleNormalTexture = AssetDatabase::GetAsset<Texture>("rifle_normal");
	shared_ptr<Texture> rifleMetallicTexture = AssetDatabase::GetAsset<Texture>("rifle_metallic");

	barrelTexture->Upload();
	barrelNormalTexture->Upload();
	barrelMetallicTexture->Upload();
	rifleTexture->Upload();
	rifleNormalTexture->Upload();
	rifleMetallicTexture->Upload();

	barrelMaterial->SetTexture("AlbedoRoughnessTex", barrelTexture);
	barrelMaterial->SetTexture("NormalTex", barrelNormalTexture);
	barrelMaterial->SetTexture("MetallicTex", barrelMetallicTexture);
	rifleMaterial->SetTexture("AlbedoRoughnessTex", rifleTexture);
	rifleMaterial->SetTexture("NormalTex", rifleNormalTexture);
	rifleMaterial->SetTexture("MetallicTex", rifleMetallicTexture);

	cube = shared_ptr<MeshRenderer>(new MeshRenderer("Cube"));
	cube->m_Mesh = cubeMesh;
	cube->m_Material = defaultMaterial;
	cube->LocalPosition(0, -.1f, 0);
	cube->LocalScale(10.0f, .1f, 10.0f);

	rifle = shared_ptr<MeshRenderer>(new MeshRenderer("Rifle"));
	rifle->m_Mesh = rifleMesh;
	rifle->m_Material = rifleMaterial;
	rifle->Parent(camera);
	rifle->LocalScale(.6f, .6f, .6f);
	rifle->LocalPosition(.21f, -.3f, .2f);

	rifle2 = shared_ptr<MeshRenderer>(new MeshRenderer("Rifle"));
	rifle2->m_Mesh = rifleMesh;
	rifle2->m_Material = rifleMaterial;
	rifle2->LocalScale(.6f, .6f, .6f);
	rifle2->LocalPosition(0, 2, 0);

	barrel = shared_ptr<MeshRenderer>(new MeshRenderer("Barrel"));
	barrel->m_Mesh = barrelMesh;
	barrel->m_Material = barrelMaterial;
	barrel->LocalPosition(0, .574f, 0);
	barrel->LocalScale(2.75f, 2.75f, 2.75f);

	dragon = shared_ptr<MeshRenderer>(new MeshRenderer("Dragon"));
	dragon->m_Mesh = dragonMesh;
	dragon->m_Material = defaultMaterial;
	dragon->LocalPosition(1, 0, 1);
	dragon->LocalRotation(XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(25.f), 0.f));
}
Game::~Game() {}

void Game::OnResize() {
	auto window = Graphics::GetWindow();
	camera->Aspect((float)window->GetWidth() / (float)window->GetHeight());
}

void Game::Update(double total, double delta) {
	// TODO switched to RH coordinate system - everything is backwards
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
		pitch += md.y * .0025f;
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
	XMVECTOR d = XMVectorZero();
	if (Input::KeyDown(KeyCode::W))
		d += fwd;
	if (Input::KeyDown(KeyCode::S))
		d -= fwd;
	if (Input::KeyDown(KeyCode::D))
		d += right;
	if (Input::KeyDown(KeyCode::A))
		d -= right;
	if (!XMVector3Equal(d, XMVectorZero()))
		camera->LocalPosition(camera->LocalPosition() + XMVector3Normalize(d) * 1.4f * (float)delta);
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

void Game::Render(shared_ptr<CommandList> commandList) {
	auto window = Graphics::GetWindow();
	auto rtv = window->GetCurrentRenderTargetView();
	auto dsv = window->GetDepthStencilView();

	DirectX::XMFLOAT4 clearColor = { 0.4f, 0.6f, 0.9f, 1.f };
	commandList->D3DCommandList()->ClearRenderTargetView(rtv, (float*)&clearColor, 0, nullptr);
	commandList->D3DCommandList()->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	commandList->SetCamera(camera);
	cube->Draw(commandList);
	dragon->Draw(commandList);
	rifle->Draw(commandList);
	rifle2->Draw(commandList);
	barrel->Draw(commandList);
}