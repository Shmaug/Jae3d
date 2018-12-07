#include "VoxelGame.hpp"

#include <Jae3d/jae.hpp>
#include <Jae3d/Profiler.hpp>
#include <Jae3d/Input.hpp>
#include <Jae3d/Util.hpp>
#include <Jae3d/Window.hpp>
#include <Jae3d/Graphics.hpp>
#include <Jae3d/CommandQueue.hpp>
#include <Jae3d/CommandList.hpp>
#include <Jae3d/AssetDatabase.hpp>

#include <Jae3d/MeshRenderer.hpp>
#include <Jae3d/Mesh.hpp>
#include <Jae3d/Camera.hpp>
#include <Jae3d/Shader.hpp>
#include <Jae3d/Material.hpp>
#include <Jae3d/Texture.hpp>
#include <Jae3d/ConstantBuffer.hpp>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;


const char* formatToString(DXGI_FORMAT f) {
	static const char* formats[120] = {
		"UNKNOWN",
		"R32G32B32A32_TYPELESS",
		"R32G32B32A32_FLOAT",
		"R32G32B32A32_UINT",
		"R32G32B32A32_SINT",
		"R32G32B32_TYPELESS",
		"R32G32B32_FLOAT",
		"R32G32B32_UINT",
		"R32G32B32_SINT",
		"R16G16B16A16_TYPELESS",
		"R16G16B16A16_FLOAT",
		"R16G16B16A16_UNORM",
		"R16G16B16A16_UINT",
		"R16G16B16A16_SNORM",
		"R16G16B16A16_SINT",
		"R32G32_TYPELESS",
		"R32G32_FLOAT",
		"R32G32_UINT",
		"R32G32_SINT",
		"R32G8X24_TYPELESS",
		"D32_FLOAT_S8X24_UINT",
		"R32_FLOAT_X8X24_TYPELESS",
		"X32_TYPELESS_G8X24_UINT",
		"R10G10B10A2_TYPELESS",
		"R10G10B10A2_UNORM",
		"R10G10B10A2_UINT",
		"R11G11B10_FLOAT",
		"R8G8B8A8_TYPELESS",
		"R8G8B8A8_UNORM",
		"R8G8B8A8_UNORM_SRGB",
		"R8G8B8A8_UINT",
		"R8G8B8A8_SNORM",
		"R8G8B8A8_SINT",
		"R16G16_TYPELESS",
		"R16G16_FLOAT",
		"R16G16_UNORM",
		"R16G16_UINT",
		"R16G16_SNORM",
		"R16G16_SINT",
		"R32_TYPELESS",
		"D32_FLOAT",
		"R32_FLOAT",
		"R32_UINT",
		"R32_SINT",
		"R24G8_TYPELESS",
		"D24_UNORM_S8_UINT",
		"R24_UNORM_X8_TYPELESS",
		"X24_TYPELESS_G8_UINT",
		"R8G8_TYPELESS",
		"R8G8_UNORM",
		"R8G8_UINT",
		"R8G8_SNORM",
		"R8G8_SINT",
		"R16_TYPELESS",
		"R16_FLOAT",
		"D16_UNORM",
		"R16_UNORM",
		"R16_UINT",
		"R16_SNORM",
		"R16_SINT",
		"R8_TYPELESS",
		"R8_UNORM",
		"R8_UINT",
		"R8_SNORM",
		"R8_SINT",
		"A8_UNORM",
		"R1_UNORM",
		"R9G9B9E5_SHAREDEXP",
		"R8G8_B8G8_UNORM",
		"G8R8_G8B8_UNORM",
		"BC1_TYPELESS",
		"BC1_UNORM",
		"BC1_UNORM_SRGB",
		"BC2_TYPELESS",
		"BC2_UNORM",
		"BC2_UNORM_SRGB",
		"BC3_TYPELESS",
		"BC3_UNORM",
		"BC3_UNORM_SRGB",
		"BC4_TYPELESS",
		"BC4_UNORM",
		"BC4_SNORM",
		"BC5_TYPELESS",
		"BC5_UNORM",
		"BC5_SNORM",
		"B5G6R5_UNORM",
		"B5G5R5A1_UNORM",
		"B8G8R8A8_UNORM",
		"B8G8R8X8_UNORM",
		"R10G10B10_XR_BIAS_A2_UNORM",
		"B8G8R8A8_TYPELESS",
		"B8G8R8A8_UNORM_SRGB",
		"B8G8R8X8_TYPELESS",
		"B8G8R8X8_UNORM_SRGB",
		"BC6H_TYPELESS",
		"BC6H_UF16",
		"BC6H_SF16",
		"BC7_TYPELESS",
		"BC7_UNORM",
		"BC7_UNORM_SRGB",
		"AYUV",
		"Y410",
		"Y416",
		"NV12",
		"P010",
		"P016",
		"420_OPAQUE",
		"YUY2",
		"Y210",
		"Y216",
		"NV11",
		"AI44",
		"IA44",
		"P8",
		"A8P8",
		"B4G4R4A4_UNORM",
		"P208",
		"V208",
		"V408",
		"FORCE_UINT"
	};
	int i = (int)f;
	if (i == 0xffffffff)
		return formats[119];
	if (i >= 130) i -= 14;
	return formats[i];
}

struct LightBuffer {
	XMFLOAT4 LightPos0;
	XMFLOAT4 LightCol0;
};
LightBuffer light;

shared_ptr<ConstantBuffer> lightBuffer;
shared_ptr<Camera> camera;
shared_ptr<MeshRenderer> cube;
shared_ptr<MeshRenderer> barrel;
shared_ptr<MeshRenderer> rifle;
shared_ptr<MeshRenderer> rifle2;
shared_ptr<MeshRenderer> dragon;

float yaw;
float pitch;

bool cursorVisible = true;

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
	HWND hWnd = JaeCreateWindow(L"Voxel Game", 1600, 900, 3);
	Graphics::GetWindow()->SetVSync(false);
	
	VoxelGame* game = new VoxelGame();
	game->Initialize();

	JaeMsgLoop(game);

	JaeDestroy();

	delete game;
	return 0;
}

void VoxelGame::Initialize() {
	AssetDatabase::LoadAssets(L"models.asset");
	AssetDatabase::LoadAssets(L"shaders.asset");
	AssetDatabase::LoadAssets(L"textures.asset");

	camera = shared_ptr<Camera>(new Camera(L"Camera"));
	camera->LocalPosition(0, 1.668f, -2.0f);
	camera->FieldOfView(60);
	auto window = Graphics::GetWindow();
	camera->Aspect((float)window->GetWidth() / (float)window->GetHeight());

	lightBuffer = shared_ptr<ConstantBuffer>(new ConstantBuffer(sizeof(LightBuffer), L"Light CB", Graphics::BufferCount()));

	shared_ptr<Shader> shader = AssetDatabase::GetAsset<Shader>(L"default");
	shared_ptr<Shader> textured = AssetDatabase::GetAsset<Shader>(L"textured");

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

	cubeMesh->Upload();
	barrelMesh->Upload();
	rifleMesh->Upload();
	dragonMesh->Upload();

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

	defaultMaterial->SetCBuffer(L"LightBuffer", lightBuffer, -1);
	barrelMaterial->SetCBuffer(L"LightBuffer", lightBuffer, -1);
	rifleMaterial->SetCBuffer(L"LightBuffer", lightBuffer, -1);

	barrelMaterial->SetTexture(L"CombinedTex", barrelTexture, -1);
	barrelMaterial->SetTexture(L"NormalTex", barrelNormalTexture, -1);
	barrelMaterial->SetTexture(L"MetallicTex", barrelMetallicTexture, -1);
	rifleMaterial->SetTexture(L"CombinedTex", rifleTexture, -1);
	rifleMaterial->SetTexture(L"NormalTex", rifleNormalTexture, -1);
	rifleMaterial->SetTexture(L"MetallicTex", rifleMetallicTexture, -1);

	cube = shared_ptr<MeshRenderer>(new MeshRenderer(L"Cube"));
	cube->mMesh = cubeMesh;
	cube->mMaterial = defaultMaterial;
	cube->LocalPosition(0, -.1f, 0);
	cube->LocalScale(10.0f, .1f, 10.0f);

	rifle = shared_ptr<MeshRenderer>(new MeshRenderer(L"Rifle"));
	rifle->mMesh = rifleMesh;
	rifle->mMaterial = rifleMaterial;
	rifle->Parent(camera);
	rifle->LocalScale(.6f, .6f, .6f);
	rifle->LocalPosition(.21f, -.3f, .2f);

	rifle2 = shared_ptr<MeshRenderer>(new MeshRenderer(L"Rifle"));
	rifle2->mMesh = rifleMesh;
	rifle2->mMaterial = rifleMaterial;
	rifle2->LocalScale(.6f, .6f, .6f);
	rifle2->LocalPosition(0, 2, 0);

	barrel = shared_ptr<MeshRenderer>(new MeshRenderer(L"Barrel"));
	barrel->mMesh = barrelMesh;
	barrel->mMaterial = barrelMaterial;
	barrel->LocalPosition(0, .574f, 0);
	barrel->LocalScale(2.75f, 2.75f, 2.75f);

	dragon = shared_ptr<MeshRenderer>(new MeshRenderer(L"Dragon"));
	dragon->mMesh = dragonMesh;
	dragon->mMaterial = defaultMaterial;
	dragon->LocalPosition(1, 0, 1);
	dragon->LocalRotation(XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(25.f), 0.f));

	light.LightCol0 = XMFLOAT4(1.5f, 1.5f, 1.5f, 1);
	XMStoreFloat4(&light.LightPos0, XMVector3Normalize(XMVectorSet(-.35f, -.6f, .25f, 1)));
}
VoxelGame::~VoxelGame() {}

void VoxelGame::OnResize() {
	auto window = Graphics::GetWindow();
	camera->Aspect((float)window->GetWidth() / (float)window->GetHeight());
}

void VoxelGame::Update(double total, double delta) {
	auto window = Graphics::GetWindow();
	if (Input::OnKeyDown(KeyCode::Enter) && Input::KeyDown(KeyCode::AltKey))
		window->SetFullscreen(!window->IsFullscreen());
	if (Input::OnKeyDown(KeyCode::F4) && Input::KeyDown(KeyCode::AltKey))
		window->Close();

	if (Input::OnKeyDown(KeyCode::R)) {
		AssetDatabase::LoadAssets(L"shaders.asset");
		shared_ptr<Shader> shader = AssetDatabase::GetAsset<Shader>(L"default");
		shared_ptr<Shader> textured = AssetDatabase::GetAsset<Shader>(L"textured");

		dragon->mMaterial->SetShader(shader);
		cube->mMaterial->SetShader(shader);
		barrel->mMaterial->SetShader(textured);
		rifle->mMaterial->SetShader(textured);
		rifle2->mMaterial->SetShader(textured);
	}

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

	lightBuffer->Write(&light, sizeof(LightBuffer), 0, -1);

	// fps stats
	static double timer = 0;
	timer += delta;
	if (timer >= 1.0) {
		timer = 0;
		OutputDebugf(L"FPS: %d.%d\n", (int)mfps, (int)((mfps - floor(mfps)) * 10.0f + .5f));

		wchar_t pbuf[1024];
		Profiler::PrintLastFrame(pbuf, 1024);
		OutputDebugString(pbuf);
	}
}
void VoxelGame::Render(shared_ptr<CommandList> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv) {
	auto window = Graphics::GetWindow();
	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.f, 0.f, (float)window->GetWidth(), (float)window->GetHeight());
	D3D12_RECT sr = { 0, 0, window->GetWidth(), window->GetHeight() };
	commandList->D3DCommandList()->RSSetViewports(1, &vp);
	commandList->D3DCommandList()->RSSetScissorRects(1, &sr);
	commandList->D3DCommandList()->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	DirectX::XMFLOAT4 clearColor = { 0.4f, 0.6f, 0.9f, 1.f };
	commandList->D3DCommandList()->ClearRenderTargetView(rtv, (float*)&clearColor, 0, nullptr);
	commandList->D3DCommandList()->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	unsigned int frameIndex = Graphics::CurrentFrameIndex();

	commandList->SetCamera(camera);
	cube->Draw(commandList, frameIndex);
	dragon->Draw(commandList, frameIndex);
	rifle->Draw(commandList, frameIndex);
	rifle2->Draw(commandList, frameIndex);
	barrel->Draw(commandList, frameIndex);
}

void VoxelGame::DoFrame(){
	static std::chrono::high_resolution_clock clock;
	static auto start = clock.now();
	static auto t0 = clock.now();
	static int frameCounter;
	static double elapsedSeconds;

	Profiler::FrameStart();

#pragma region update
	auto t1 = clock.now();
	double delta = (t1 - t0).count() * 1e-9;
	t0 = t1;

	Profiler::BeginSample("Update");
	Update((t1 - start).count() * 1e-9, delta);
	Profiler::EndSample();
#pragma endregion

#pragma region render
	Profiler::BeginSample("Render");
	auto commandQueue = Graphics::GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList(Graphics::CurrentFrameIndex());
	auto d3dCommandList = commandList->D3DCommandList();
	auto window = Graphics::GetWindow();

	window->PrepareRenderTargets(commandList);

	auto rtv = window->GetCurrentRenderTargetView();
	auto dsv = window->GetDepthStencilView();

	Render(commandList, rtv, dsv);
	Profiler::EndSample();

	Profiler::BeginSample("Present");
	window->Present(commandList, commandQueue);

	Profiler::EndSample();
#pragma endregion

	// measure fps
	frameCounter++;
	elapsedSeconds += delta;
	if (elapsedSeconds > 1.0) {
		mfps = frameCounter / elapsedSeconds;
		frameCounter = 0;
		elapsedSeconds = 0.0;
	}

	Input::FrameEnd();
	Profiler::FrameEnd();
}