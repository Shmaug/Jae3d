#include "TestGame.hpp"

#include <jae.hpp>
#include <Profiler.hpp>
#include <Input.hpp>
#include <Util.hpp>
#include <Window.hpp>
#include <Graphics.hpp>
#include <CommandQueue.hpp>
#include <CommandList.hpp>
#include <AssetDatabase.hpp>
#include <Scene.hpp>

#include <MeshRenderer.hpp>
#include <Mesh.hpp>
#include <Camera.hpp>
#include <Light.hpp>
#include <Shader.hpp>
#include <Material.hpp>
#include <Texture.hpp>
#include <ConstantBuffer.hpp>
#include <SpriteBatch.hpp>
#include <Font.hpp>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

shared_ptr<Camera> camera;
shared_ptr<Scene> scene;
shared_ptr<Font> arial;

wchar_t pbuf[1024];

float frameTimes[128];
unsigned int frameTimeIndex;

float yaw;
float pitch;
bool debugDraw = false;
bool wireframe = false;
bool cursorVisible = true;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
	HWND hWnd = JaeCreateWindow(L"Jae3d Test", 1600, 900, 3);
	Graphics::GetWindow()->SetVSync(false);
	
	TestGame* game = new TestGame();
	game->Initialize();

	JaeMsgLoop(game);

	JaeDestroy();

	delete game;
	return 0;
}

void TestGame::Initialize() {
	AssetDatabase::LoadAssets(L"core.asset");
	AssetDatabase::LoadAssets(L"models.asset");
	AssetDatabase::LoadAssets(L"shaders.asset");
	AssetDatabase::LoadAssets(L"textures.asset");

	scene = make_shared<Scene>();

	arial = AssetDatabase::GetAsset<Font>(L"arial");
	arial->GetTexture()->Upload();

	camera = scene->AddObject<Camera>(L"Camera");
	camera->LocalPosition(0, 1.668f, -2.0f);
	camera->FieldOfView(60);
	camera->PixelWidth(Graphics::GetWindow()->GetWidth());
	camera->PixelHeight(Graphics::GetWindow()->GetHeight());

	shared_ptr<Shader> shader = AssetDatabase::GetAsset<Shader>(L"Default");
	shared_ptr<Shader> textured = AssetDatabase::GetAsset<Shader>(L"Textured");

	shared_ptr<Mesh> cubeMesh = shared_ptr<Mesh>(new Mesh(L"Cube"));
	shared_ptr<Mesh> barrelMesh = AssetDatabase::GetAsset<Mesh>(L"Barrel");
	shared_ptr<Mesh> rifleMesh = AssetDatabase::GetAsset<Mesh>(L"Rifle.Rifle");
	shared_ptr<Mesh> dragonMesh = AssetDatabase::GetAsset<Mesh>(L"Dragon");

	shared_ptr<Material> defaultMaterial = shared_ptr<Material>(new Material(L"Default", shader));
	shared_ptr<Material> barrelMaterial = shared_ptr<Material>(new Material(L"Barrel", textured));
	shared_ptr<Material> rifleMaterial = shared_ptr<Material>(new Material(L"Rifle", textured));

	shared_ptr<Texture> uvgridTexture = AssetDatabase::GetAsset<Texture>(L"uvgrid");
	shared_ptr<Texture> barrelTexture = AssetDatabase::GetAsset<Texture>(L"barrel_combined");
	shared_ptr<Texture> barrelNormalTexture = AssetDatabase::GetAsset<Texture>(L"barrel_normal");
	shared_ptr<Texture> barrelMetallicTexture = AssetDatabase::GetAsset<Texture>(L"barrel_metallic");
	shared_ptr<Texture> rifleTexture = AssetDatabase::GetAsset<Texture>(L"rifle_combined");
	shared_ptr<Texture> rifleNormalTexture = AssetDatabase::GetAsset<Texture>(L"rifle_normal");
	shared_ptr<Texture> rifleMetallicTexture = AssetDatabase::GetAsset<Texture>(L"rifle_metallic");

	cubeMesh->LoadCube(1.0f);

	cubeMesh->UploadStatic();
	barrelMesh->UploadStatic();
	rifleMesh->UploadStatic();
	dragonMesh->UploadStatic();

	uvgridTexture->Upload();
	barrelTexture->Upload();
	barrelNormalTexture->Upload();
	barrelMetallicTexture->Upload();
	rifleTexture->Upload();
	rifleNormalTexture->Upload();
	rifleMetallicTexture->Upload();

	rifleMaterial->SetFloat(L"roughness", 1.0f, -1);
	rifleMaterial->SetFloat(L"metallic", 1.0f, -1);
	barrelMaterial->SetFloat(L"roughness", 1.0f, -1);
	barrelMaterial->SetFloat(L"metallic", 1.0f, -1);

	barrelMaterial->SetTexture(L"CombinedTex", barrelTexture, -1);
	barrelMaterial->SetTexture(L"NormalTex", barrelNormalTexture, -1);
	barrelMaterial->SetTexture(L"MetallicTex", barrelMetallicTexture, -1);
	rifleMaterial->SetTexture(L"CombinedTex", rifleTexture, -1);
	rifleMaterial->SetTexture(L"NormalTex", rifleNormalTexture, -1);
	rifleMaterial->SetTexture(L"MetallicTex", rifleMetallicTexture, -1);

	auto cube = scene->AddObject<MeshRenderer>(L"Cube");
	cube->mMesh = cubeMesh;
	cube->mMaterial = defaultMaterial;
	cube->LocalPosition(0, -.1f, 0);
	cube->LocalScale(50.0f, .1f, 50.0f);

	auto rifle = scene->AddObject<MeshRenderer>(L"Rifle");
	rifle->mMesh = rifleMesh;
	rifle->mMaterial = rifleMaterial;
	rifle->Parent(camera);
	rifle->LocalScale(.6f, .6f, .6f);
	rifle->LocalPosition(.21f, -.3f, .2f);

	for (unsigned int x = 0; x < 16; x++) {
		for (unsigned int y = 0; y < 16; y++) {
			auto barrel = scene->AddObject<MeshRenderer>(L"Barrel");
			barrel->mMesh = barrelMesh;
			barrel->mMaterial = barrelMaterial;
			barrel->LocalPosition((x - 7.5f) * 1.5f, .574f, (y - 7.5f) * 1.5f);
			barrel->LocalScale(2.75f, 2.75f, 2.75f);
		}
	}

	auto dragon = scene->AddObject<MeshRenderer>(L"Dragon");
	dragon->mMesh = dragonMesh;
	dragon->mMaterial = defaultMaterial;
	dragon->LocalScale(2, 2, 2);

	for (unsigned int x = 0; x < 8; x++) {
		for (unsigned int y = 0; y < 8; y++) {
			auto light = scene->AddObject<Light>(L"Light");
			light->LocalPosition(3.0f * (float)(x - 3.5f), 1.25f, 3.0f * (float)(y - 3.5f));
			light->mColor = { (float)x * .125f, (float)y * .125f, (float)(x + y) * .125f };
			light->mIntensity = 5.0f;
			light->mRange = 2.0f;
		}
	}
}
TestGame::~TestGame() {}

void TestGame::OnResize() {
	auto window = Graphics::GetWindow();
	camera->PixelWidth(window->GetWidth());
	camera->PixelHeight(window->GetHeight());
}

void TestGame::Update(double total, double delta) {
	auto window = Graphics::GetWindow();
	if (Input::OnKeyDown(KeyCode::Enter) && Input::KeyDown(KeyCode::AltKey))
		window->SetFullscreen(!window->IsFullscreen());
	if (Input::OnKeyDown(KeyCode::F4) && Input::KeyDown(KeyCode::AltKey))
		window->Close();

	if (Input::OnKeyDown(KeyCode::F1))
		wireframe = !wireframe;
	if (Input::OnKeyDown(KeyCode::F2))
		debugDraw = !debugDraw;

	if (Input::KeyDown(KeyCode::H))
		Sleep(10);
	if (Input::KeyDown(KeyCode::J))
		Sleep(5);

#pragma region camera controls
	if (Input::OnKeyDown(KeyCode::AltKey))
		Input::mLockMouse = !Input::mLockMouse;

	if (Input::mLockMouse) {
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

	XMVECTOR rotation = XMLoadFloat4(&camera->LocalRotation());
	XMVECTOR right = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), rotation);
	XMVECTOR up    = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rotation);
	XMVECTOR fwd   = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rotation);
	XMVECTOR d = XMVectorZero();
	if (Input::KeyDown(KeyCode::W))
		d += fwd;
	if (Input::KeyDown(KeyCode::S))
		d -= fwd;
	if (Input::KeyDown(KeyCode::D))
		d += right;
	if (Input::KeyDown(KeyCode::A))
		d -= right;
	if (Input::KeyDown(KeyCode::E))
		d += up;
	if (Input::KeyDown(KeyCode::Q))
		d -= up;
	if (!XMVector3Equal(d, XMVectorZero())) {
		float s = 1.4f;
		if (Input::KeyDown(KeyCode::ShiftKey))
			s *= 2;
		camera->LocalPosition(XMLoadFloat3(&camera->LocalPosition()) + XMVector3Normalize(d) * s * (float)delta);
	}
#pragma endregion
}

void TestGame::Render(shared_ptr<CommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv) {
	auto window = Graphics::GetWindow();
	float w = (float)window->GetWidth();
	float h = (float)window->GetHeight();

	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.f, 0.f, w, h);
	D3D12_RECT sr = { 0, 0, window->GetWidth(), window->GetHeight() };
	commandList->D3DCommandList()->RSSetViewports(1, &vp);
	commandList->D3DCommandList()->RSSetScissorRects(1, &sr);
	commandList->D3DCommandList()->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	DirectX::XMFLOAT4 clearColor = { 0.2f, 0.2f, 0.2f, 1.f };
	commandList->D3DCommandList()->ClearRenderTargetView(rtv, (float*)&clearColor, 0, nullptr);
	commandList->D3DCommandList()->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	if (wireframe) commandList->SetFillMode(D3D12_FILL_MODE_WIREFRAME);
	scene->Draw(commandList, camera);

	if (debugDraw) scene->DebugDraw(commandList, camera);

	#pragma region performance overlay
	commandList->SetFillMode(D3D12_FILL_MODE_SOLID);
	shared_ptr<SpriteBatch> sb = Graphics::GetSpriteBatch();
	sb->DrawTextf(arial, XMFLOAT2(10.0f, (float)arial->GetAscender() * .5f), .5f, {1,1,1,1}, L"FPS: %d.%d\n", (int)mfps, (int)((mfps - floor(mfps)) * 10.0f + .5f));
	sb->DrawTextf(arial, XMFLOAT2(10.0f, (float)arial->GetAscender()), .5f, { 1,1,1,1 }, pbuf);

	jvector<XMFLOAT3> verts;
	jvector<XMFLOAT4> colors;
	for (int i = 1; i < 128; i++) {
		int d = frameTimeIndex - i; if (d < 0) d += 128;
		verts.push_back({ 512.0f - i * 4.0f, h - frameTimes[d] * 5000, 0 });
		float r = fmin(frameTimes[d] / .025f, 1.0f); // full red < 40fps
		r *= r;
		colors.push_back({ r, 1.0f - r, 0, 1 });
	}
	sb->DrawLines(verts, colors);

	sb->Flush(commandList);
	#pragma endregion
}

void TestGame::DoFrame(){
	static std::chrono::high_resolution_clock clock;
	static auto start = clock.now();
	static auto t0 = clock.now();
	static int frameCounter;
	static double elapsedSeconds;
	static double elapsedSeconds2;

	Profiler::FrameStart();

#pragma region update
	Profiler::BeginSample(L"Update");
	auto t1 = clock.now();
	double delta = (t1 - t0).count() * 1e-9;
	t0 = t1;
	Update((t1 - start).count() * 1e-9, delta);
	Profiler::EndSample();
#pragma endregion

#pragma region render
	Profiler::BeginSample(L"Render");
	auto commandQueue = Graphics::GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList(Graphics::CurrentFrameIndex());
	auto d3dCommandList = commandList->D3DCommandList();
	auto window = Graphics::GetWindow();

	window->PrepareRenderTargets(commandList);

	auto rtv = window->GetCurrentRenderTargetView();
	auto dsv = window->GetDepthStencilView();

	Render(commandList, rtv, dsv);
	Profiler::EndSample();

	Profiler::BeginSample(L"Present");
	window->Present(commandList, commandQueue);

	Profiler::EndSample();
#pragma endregion

	Input::FrameEnd();
	Profiler::FrameEnd();

	// measure fps
	frameCounter++;
	elapsedSeconds += delta;
	if (elapsedSeconds > 1.0) {
		mfps = frameCounter / elapsedSeconds;
		frameCounter = 0;
		elapsedSeconds = 0.0;
		ZeroMemory(pbuf, sizeof(wchar_t) * 1024);
		Profiler::PrintLastFrame(pbuf, 1024);
	}

	elapsedSeconds2 += delta;
	if (elapsedSeconds2 > 0.05) {
		elapsedSeconds2 = 0;
		frameTimes[frameTimeIndex] = (float)Profiler::LastFrameTime();
		frameTimeIndex = (frameTimeIndex + 1) % 128;
	}
}