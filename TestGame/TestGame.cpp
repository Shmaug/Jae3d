#include "TestGame.hpp"

#include <jae.hpp>
#include <Profiler.hpp>
#include <Input.hpp>
#include <Util.hpp>
#include <Window.hpp>
#include <Graphics.hpp>
#include <CommandQueue.hpp>
#include <CommandList.hpp>

#include <MeshRenderer.hpp>
#include <Mesh.hpp>
#include <Camera.hpp>
#include <Light.hpp>
#include <Shader.hpp>
#include <Material.hpp>
#include <Texture.hpp>
#include <DescriptorTable.hpp>
#include <ConstantBuffer.hpp>
#include <SpriteBatch.hpp>

#include "Lighting.hpp"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

#define F2D(x) (int)x, (int)(abs(x - (int)x)*1000)

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
	mLoadingOperations.push_back(AssetDatabase::LoadAssetsAsync(L"models.asset"));
	mLoadingOperations.push_back(AssetDatabase::LoadAssetsAsync(L"shaders.asset"));
	mLoadingOperations.push_back(AssetDatabase::LoadAssetsAsync(L"textures.asset"));

	AssetDatabase::LoadAssets(L"core.asset");
	mArialFont = AssetDatabase::GetAsset<Font>(L"arial");
	mArialFont->GetTexture()->Upload();

	mScene = make_shared<Scene>();

	mCamera = mScene->AddObject<Camera>(L"Camera");
	mCamera->LocalPosition(0, 1.668f, -2.0f);
	mCamera->FieldOfView(XMConvertToRadians(60));
	mCamera->SampleCount(8);
	mCamera->CreateRenderBuffers();
}
TestGame::~TestGame() {}

void TestGame::InitializeScene() {
	shared_ptr<Shader> shader = AssetDatabase::GetAsset<Shader>(L"Default");
	shared_ptr<Shader> textured = AssetDatabase::GetAsset<Shader>(L"Textured");
	shared_ptr<Shader> skybox = AssetDatabase::GetAsset<Shader>(L"Skybox");

	mLighting = make_shared<Lighting>();

	#pragma region Textures
	shared_ptr<Texture> uvgridTexture = AssetDatabase::GetAsset<Texture>(L"uvgrid");
	shared_ptr<Texture> barrelTexture = AssetDatabase::GetAsset<Texture>(L"barrel_combined");
	shared_ptr<Texture> barrelNormalTexture = AssetDatabase::GetAsset<Texture>(L"barrel_normal");
	shared_ptr<Texture> barrelMetallicTexture = AssetDatabase::GetAsset<Texture>(L"barrel_metallic");
	shared_ptr<Texture> rifleTexture = AssetDatabase::GetAsset<Texture>(L"rifle_combined");
	shared_ptr<Texture> rifleNormalTexture = AssetDatabase::GetAsset<Texture>(L"rifle_normal");
	shared_ptr<Texture> rifleMetallicTexture = AssetDatabase::GetAsset<Texture>(L"rifle_metallic");
	shared_ptr<Texture> a7eTexture = AssetDatabase::GetAsset<Texture>(L"a7e_albedo");
	shared_ptr<Texture> a7eMetallicTexture = AssetDatabase::GetAsset<Texture>(L"a7e_metallic");
	shared_ptr<Texture> seatTexture = AssetDatabase::GetAsset<Texture>(L"seat_combined");
	shared_ptr<Texture> seatNormalTexture = AssetDatabase::GetAsset<Texture>(L"seat_normal");
	shared_ptr<Texture> seatMetallicTexture = AssetDatabase::GetAsset<Texture>(L"seat_metallic");
	shared_ptr<Texture> tankTexture = AssetDatabase::GetAsset<Texture>(L"tank_combined");
	shared_ptr<Texture> tankNormalTexture = AssetDatabase::GetAsset<Texture>(L"tank_normal");
	shared_ptr<Texture> tankMetallicTexture = AssetDatabase::GetAsset<Texture>(L"tank_metallic");
	shared_ptr<Texture> skyTexture = AssetDatabase::GetAsset<Texture>(L"sky");

	uint8_t bumpColor[4]{ 0x7F, 0x7F, 0xFF, 0xFF };
	uint8_t whiteColor[4]{ 0xFF, 0xFF, 0xFF, 0xFF };
	uint8_t blackColor[4]{ 0x00, 0x00, 0x00, 0xFF };
	shared_ptr<Texture> bumpTexture  = shared_ptr<Texture>(new Texture(L"bump" , 1, 1, 1, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 1, bumpColor , 4, false));
	shared_ptr<Texture> whiteTexture = shared_ptr<Texture>(new Texture(L"white", 1, 1, 1, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 1, whiteColor, 4, false));
	shared_ptr<Texture> blackTexture = shared_ptr<Texture>(new Texture(L"black", 1, 1, 1, D3D12_RESOURCE_DIMENSION_TEXTURE2D, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 1, blackColor, 4, false));

	uvgridTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	barrelTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	barrelNormalTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	barrelMetallicTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	rifleTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	rifleNormalTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	rifleMetallicTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	a7eTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	a7eMetallicTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	seatTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	seatNormalTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	seatMetallicTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	tankTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	tankNormalTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	tankMetallicTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	bumpTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	whiteTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	blackTexture->Upload(D3D12_RESOURCE_FLAG_NONE, false);
	skyTexture->Upload();
	#pragma endregion

	#pragma region Meshes
	shared_ptr<Mesh> cubeMesh = shared_ptr<Mesh>(new Mesh(L"Cube"));
	shared_ptr<Mesh> barrelMesh = AssetDatabase::GetAsset<Mesh>(L"Barrel");
	shared_ptr<Mesh> rifleMesh = AssetDatabase::GetAsset<Mesh>(L"Rifle");
	shared_ptr<Mesh> dragonMesh = AssetDatabase::GetAsset<Mesh>(L"Dragon");
	shared_ptr<Mesh> a7eMesh = AssetDatabase::GetAsset<Mesh>(L"A7E Corsair II");
	shared_ptr<Mesh> a7eSeatMesh = AssetDatabase::GetAsset<Mesh>(L"Seat");
	shared_ptr<Mesh> a7eTankMesh = AssetDatabase::GetAsset<Mesh>(L"DropTank");

	cubeMesh->LoadCube(1.0f);

	cubeMesh->UploadStatic();
	barrelMesh->UploadStatic();
	rifleMesh->UploadStatic();
	dragonMesh->UploadStatic();
	a7eMesh->UploadStatic();
	a7eSeatMesh->UploadStatic();
	a7eTankMesh->UploadStatic();
	#pragma endregion

	#pragma region Materials
	shared_ptr<Material> defaultMaterial = shared_ptr<Material>(new Material(L"Default", shader));
	shared_ptr<Material> barrelMaterial = shared_ptr<Material>(new Material(L"Barrel", textured));
	shared_ptr<Material> rifleMaterial = shared_ptr<Material>(new Material(L"Rifle", textured));
	shared_ptr<Material> a7eMaterial = shared_ptr<Material>(new Material(L"A7E", textured));
	shared_ptr<Material> a7eSeatMaterial = shared_ptr<Material>(new Material(L"Seat", textured));
	shared_ptr<Material> a7eTankMaterial = shared_ptr<Material>(new Material(L"Tank", textured));
	shared_ptr<Material> redLightMaterial = shared_ptr<Material>(new Material(L"Red Light", shader));
	shared_ptr<Material> glassMaterial = shared_ptr<Material>(new Material(L"Glass", textured));
	shared_ptr<Material> skyboxMaterial = shared_ptr<Material>(new Material(L"Skybox", skybox));

	shared_ptr<DescriptorTable> barrelTable = shared_ptr<DescriptorTable>(new DescriptorTable(3));
	barrelTable->SetSRV(0, barrelTexture);
	barrelTable->SetSRV(1, barrelNormalTexture);
	barrelTable->SetSRV(2, barrelMetallicTexture);
	barrelMaterial->SetDescriptorTable("MaterialTextures", barrelTable, -1);
	shared_ptr<DescriptorTable> rifleTable = shared_ptr<DescriptorTable>(new DescriptorTable(3));
	rifleTable->SetSRV(0, rifleTexture);
	rifleTable->SetSRV(1, rifleNormalTexture);
	rifleTable->SetSRV(2, rifleMetallicTexture);
	rifleMaterial->SetDescriptorTable("MaterialTextures", rifleTable, -1);

	shared_ptr<DescriptorTable> a7eTable = shared_ptr<DescriptorTable>(new DescriptorTable(3));
	a7eTable->SetSRV(0, a7eTexture);
	a7eTable->SetSRV(1, bumpTexture);
	a7eTable->SetSRV(2, a7eMetallicTexture);
	a7eMaterial->SetDescriptorTable("MaterialTextures", a7eTable, -1);
	shared_ptr<DescriptorTable> glassTable = shared_ptr<DescriptorTable>(new DescriptorTable(3));
	glassTable->SetSRV(0, a7eTexture);
	glassTable->SetSRV(1, bumpTexture);
	glassTable->SetSRV(2, blackTexture);
	glassMaterial->SetDescriptorTable("MaterialTextures", glassTable, -1);
	shared_ptr<DescriptorTable> seatTable = shared_ptr<DescriptorTable>(new DescriptorTable(3));
	seatTable->SetSRV(0, seatTexture);
	seatTable->SetSRV(1, seatNormalTexture);
	seatTable->SetSRV(2, seatMetallicTexture);
	a7eSeatMaterial->SetDescriptorTable("MaterialTextures", seatTable, -1);
	shared_ptr<DescriptorTable> tankTable = shared_ptr<DescriptorTable>(new DescriptorTable(3));
	tankTable->SetSRV(0, tankTexture);
	tankTable->SetSRV(1, tankNormalTexture);
	tankTable->SetSRV(2, tankMetallicTexture);
	a7eTankMaterial->SetDescriptorTable("MaterialTextures", tankTable, -1);

	defaultMaterial->SetFloat("metallic", 0.0f, -1);
	defaultMaterial->SetFloat("smoothness", 0.4f, -1);
	a7eMaterial->SetFloat("smoothness", .6f, -1);
	a7eSeatMaterial->SetFloat("smoothness", .5f, -1);
	a7eTankMaterial->SetFloat("smoothness", .8f, -1);
	glassMaterial->SetFloat("smoothness", .8f, -1);
	glassMaterial->SetFloat("alpha", .1f, -1);
	glassMaterial->ZWrite(false);
	glassMaterial->Blend(BLEND_STATE_ALPHA);
	glassMaterial->RenderQueue(3000);
	redLightMaterial->SetColor3("color", { 1, 0, 0 }, -1);
	redLightMaterial->SetColor3("emission", { 1, 0, 0 }, -1);

	skyboxMaterial->ZWrite(false);
	skyboxMaterial->CullMode(D3D12_CULL_MODE_FRONT);
	skyboxMaterial->SetTexture("Texture", skyTexture, -1);
	mScene->Skybox(skyboxMaterial);

	#pragma endregion

	#pragma region Objects
	auto rifle = mScene->AddObject<MeshRenderer>(L"Rifle");
	rifle->Parent(mCamera);
	rifle->SetMesh(rifleMesh);
	rifle->SetMaterial(rifleMaterial, 0);
	rifle->SetMaterial(defaultMaterial, 1); // iron sight glow
	rifle->LocalScale(.6f, .6f, .6f);
	rifle->LocalPosition(.21f, -.3f, .2f);

	auto cube = mScene->AddObject<MeshRenderer>(L"Cube");
	cube->SetMesh(cubeMesh);
	cube->SetMaterial(defaultMaterial);
	cube->LocalPosition(0, -.1f, 0);
	cube->LocalScale(50.0f, .1f, 50.0f);

	auto dragon = mScene->AddObject<MeshRenderer>(L"Dragon");
	dragon->SetMesh(dragonMesh);
	dragon->SetMaterial(defaultMaterial);
	dragon->LocalPosition(-2, 0, 0);
	dragon->LocalScale(2, 2, 2);

	auto a7e = mScene->AddObject<MeshRenderer>(L"A7E Corsair II");
	a7e->SetMesh(a7eMesh);
	a7e->SetMaterial(redLightMaterial, 0); // red lights
	a7e->SetMaterial(a7eMaterial, 1);
	a7e->SetMaterial(glassMaterial, 2); // canopy glass
	a7e->LocalPosition(7, .95, 0);
	a7e->LocalRotation(XMQuaternionRotationRollPitchYaw(XMConvertToRadians(-2.5f), 0, 0));

	auto a7eSeat = mScene->AddObject<MeshRenderer>(L"A7E Seat");
	a7eSeat->Parent(a7e);
	a7eSeat->SetMesh(a7eSeatMesh);
	a7eSeat->SetMaterial(a7eSeatMaterial);
	a7eSeat->LocalPosition(0, .01976f, 1.99565);

	auto a7eTankLeft = mScene->AddObject<MeshRenderer>(L"A7E Drop Tank");
	a7eTankLeft->Parent(a7e);
	a7eTankLeft->SetMesh(a7eTankMesh);
	a7eTankLeft->SetMaterial(a7eTankMaterial);
	a7eTankLeft->LocalPosition(-1.08257f, -0.20482f, -0.30395);

	auto a7eTankRight = mScene->AddObject<MeshRenderer>(L"A7E Drop Tank");
	a7eTankRight->Parent(a7e);
	a7eTankRight->SetMesh(a7eTankMesh);
	a7eTankRight->SetMaterial(a7eTankMaterial);
	a7eTankRight->LocalPosition(1.08257f, -0.20482f, -0.30395);

	auto barrel = mScene->AddObject<MeshRenderer>(L"Barrel");
	barrel->SetMesh(barrelMesh);
	barrel->SetMaterial(barrelMaterial);
	barrel->LocalPosition(0, .574f, 0);
	barrel->LocalScale(2.75f, 2.75f, 2.75f);
	#pragma endregion

	#pragma region Lighting
	mLighting->SetEnvironmentTexture(skyTexture);

	auto sun = mScene->AddObject<Light>(L"Light");
	sun->mShadows = true;
	sun->LocalRotation(XMQuaternionRotationRollPitchYaw(.361f, -.91939f, 0));
	sun->mMode = Light::LIGHTMODE_DIRECTIONAL;

	for (unsigned int x = 0; x < 8; x++) {
		for (unsigned int y = 0; y < 8; y++) {
			auto light = mScene->AddObject<Light>(L"Light");
			light->LocalPosition(3.0f * (float)(x - 3.5f), 1.5f, 3.0f * (float)(y - 3.5f));
			light->mColor = { (float)x * .125f, (float)y * .125f, (float)(x + y) * .125f };
			light->mIntensity = 7.0f;
			light->mRange = 2.0f;
			light->mMode = Light::LIGHTMODE_POINT;
		}
	}
#pragma endregion
}

void TestGame::OnResize() {
	auto window = Graphics::GetWindow();
	mCamera->PixelWidth(window->GetWidth());
	mCamera->PixelHeight(window->GetHeight());
	mCamera->CreateRenderBuffers();
}

void TestGame::Update(double total, double delta) {
	auto window = Graphics::GetWindow();
	if (Input::OnKeyDown(KeyCode::Enter) && Input::KeyDown(KeyCode::AltKey))
		window->SetFullscreen(!window->IsFullscreen());
	if (Input::OnKeyDown(KeyCode::F4) && Input::KeyDown(KeyCode::AltKey))
		window->Close();

	if (mLoading) {
		bool done = true;
		for (unsigned int i = 0; i < mLoadingOperations.size(); i++) {
			if (!mLoadingOperations[i]->mDone) {
				done = false;
				break;
			}
		}
		if (done) {
			for (unsigned int i = 0; i < mLoadingOperations.size(); i++)
				AssetDatabase::FinishLoadAssetsAsync(mLoadingOperations[i]);

			InitializeScene();
			mLoading = false;
		} else
			return;
	}

	if (Input::OnKeyDown(KeyCode::F1)) mPerfOverlay = !mPerfOverlay;
	if (Input::OnKeyDown(KeyCode::F2)) mDebugDraw = !mDebugDraw;
	if (Input::OnKeyDown(KeyCode::F3)) mWireframe = !mWireframe;

	#pragma region camera controls
	if (Input::OnKeyDown(KeyCode::AltKey))
		Input::mLockMouse = !Input::mLockMouse;

	if (Input::mLockMouse) {
		XMINT2 md = Input::MouseDelta();
		mFpsYaw += md.x * .0025f;
		mFpsPitch += md.y * .0025f;
		mFpsPitch = fmin(fmax(mFpsPitch, -XM_PIDIV2), XM_PIDIV2);
		mCamera->LocalRotation(XMQuaternionRotationRollPitchYaw(mFpsPitch, mFpsYaw, 0));

		if (mCursorVisible) {
			ShowCursor(false);
			mCursorVisible = false;
		}
	} else {
		if (!mCursorVisible) {
			ShowCursor(true);
			mCursorVisible = true;
		}
	}

	XMVECTOR rotation = XMLoadFloat4(&mCamera->LocalRotation());
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
		mCamera->LocalPosition(XMLoadFloat3(&mCamera->LocalPosition()) + XMVector3Normalize(d) * s * (float)delta);
	}
	#pragma endregion
}

void TestGame::Render(shared_ptr<CommandList> commandList) {
	unsigned int i = commandList->GetFrameIndex();

	shared_ptr<SpriteBatch> sb = Graphics::GetSpriteBatch();

	if (!mLoading) {
		Profiler::BeginSample(L"Calculate Tiled Lighting");
		mLighting->CalculateScreenLights(commandList, mCamera, mScene);
		Profiler::EndSample();

		commandList->SetCamera(mCamera);
		mCamera->Clear(commandList);

		mScene->DrawSkybox(commandList);
		auto envMap = mLighting->CalculateEnvironmentTexture();
		commandList->SetGlobalTexture("EnvironmentMap", envMap);
		commandList->SetGlobalFloat("envIntensity", .6f);

		if (mWireframe) commandList->SetFillMode(D3D12_FILL_MODE_WIREFRAME);

		Profiler::BeginSample(L"Draw Scene");
		mScene->Draw(commandList, mCamera);
		Profiler::EndSample();

		Profiler::BeginSample(L"Draw Scene Debug");
		if (mDebugDraw) mScene->DebugDraw(commandList, mCamera);
		Profiler::EndSample();
	} else {
		commandList->SetCamera(mCamera);
		mCamera->Clear(commandList);

		sb->DrawTextf(mArialFont, XMFLOAT2((float)commandList->CurrentRenderTargetWidth() * .5f - 100.0f, (float)commandList->CurrentRenderTargetHeight() * .5f), 1.0f, { 1,1,1,1 }, L"LOADING");
	}

#pragma region performance overlay
	sb->DrawTextf(mArialFont, XMFLOAT2(5.0f, (float)mArialFont->GetAscender() * .4f), .4f, { 1,1,1,1 }, L"FPS: %d.%d", F2D(mfps));
	if (mPerfOverlay) {
		Profiler::BeginSample(L"Performance Overlay");
		Profiler::BeginSample(L"Text");
		commandList->SetFillMode(D3D12_FILL_MODE_SOLID);
		sb->DrawTextf(mArialFont, XMFLOAT2(10.0f, (float)mArialFont->GetAscender()), .5f, { 1,1,1,1 }, mPerfBuffer);
		sb->DrawTextf(mArialFont, XMFLOAT2(500.0f, (float)mArialFont->GetAscender() * .5f), .5f, { 1,1,1,1 }, L"%d Triangles", commandList->mTrianglesDrawn);
		Profiler::EndSample();

		Profiler::BeginSample(L"Frame Graph");
		jvector<XMFLOAT3> verts;
		jvector<XMFLOAT4> colors;
		for (int i = 1; i < 128; i++) {
			int d = mFrameTimeIndex - i; if (d < 0) d += 128;
			verts.push_back({ 512.0f - i * 4.0f, (float)commandList->CurrentRenderTargetHeight() - mFrameTimes[d] * 5000, 0 });
			// full red < 30fps, mostly green > 60fps
			float r = fmin(mFrameTimes[d] / .025f, 1.0f);
			r *= r;
			r *= r;
			r *= .25f;
			colors.push_back({ r, 1.0f - r, 0, 1 });
		}
		sb->DrawLines(verts, colors);
		Profiler::EndSample();

		Profiler::EndSample();
	}
#pragma endregion
	
	sb->Flush(commandList);
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

#pragma region render/present
	Profiler::BeginSample(L"Render");
	auto commandQueue = Graphics::GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList(Graphics::CurrentFrameIndex());
	auto d3dCommandList = commandList->D3DCommandList();
	auto window = Graphics::GetWindow();

	window->PrepareRender(commandList);
	Render(commandList);
	Profiler::EndSample();

	Profiler::BeginSample(L"Present");

	ID3D12Resource* camrt = mCamera->RenderBuffer().Get();
	ID3D12Resource* winrt = window->RenderBuffer().Get();
	commandList->TransitionResource(camrt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	commandList->TransitionResource(winrt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_DEST);
	d3dCommandList->ResolveSubresource(winrt, 0, camrt, 0, mCamera->RenderFormat());
	commandList->TransitionResource(camrt, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->TransitionResource(winrt, D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

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
		ZeroMemory(mPerfBuffer, sizeof(wchar_t) * 4096);
		Profiler::PrintLastFrame(mPerfBuffer, 4096);
	}

	elapsedSeconds2 += delta;
	if (elapsedSeconds2 > 0.05) {
		elapsedSeconds2 = 0;
		mFrameTimes[mFrameTimeIndex] = (float)Profiler::LastFrameTime();
		mFrameTimeIndex = (mFrameTimeIndex + 1) % 128;
	}
}