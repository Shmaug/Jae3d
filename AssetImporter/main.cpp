#include <iostream>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "runtimeobject.lib")
#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "gdi32.lib")

#include <algorithm>

#include <Common.hpp>
#include <jae.hpp>

#include <IOUtil.hpp>
#include <AssetFile.hpp>
#include <Asset.hpp>
#include <Mesh.hpp>
#include <Shader.hpp>
#include <Texture.hpp>

#include "MeshImporter.hpp"
#include "ShaderImporter.hpp"
#include "TextureImporter.hpp"
#include "FontImporter.hpp"

#include "Label.hpp"
#include "Button.hpp"
#include "FileTree.hpp"
#include "Properties.hpp"
#include "Viewport.hpp"
#include "CDialogEventHandler.hpp"

#pragma warning(disable:4311)
#pragma warning(disable:4302)

#define ID_FILE_LOADFILE 9001
#define ID_FILE_LOADFOLDER 9002

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST ((HINSTANCE)&__ImageBase)

HFONT boldFont, font;

jvector<std::shared_ptr<UIControl>> controls;
std::shared_ptr<FileTree> fileTree;
std::shared_ptr<Viewport> viewport;
std::shared_ptr<Properties> properties;
HWND vphwnd = 0;

void LoadFile(jwstring file, jvector<AssetMetadata> &meta, jvector<jwstring> shaderIncludePaths) {
	if (PathFileExistsW(file.c_str()) != 1) {
		wprintf(L"Could not find %s\n", file.c_str());
		return;
	}

	jwstring ext = GetExtW(file);

	if (ext == L"obj")
		ImportMesh(file, meta);
	else if (ext == L"fbx")
		ImportMesh(file, meta);
	else if (ext == L"blend")
		ImportMesh(file, meta);
	
	else if (ext == L"png")
		ImportTexture(file, meta);
	else if (ext == L"gif")
		ImportTexture(file, meta);
	else if (ext == L"bmp")
		ImportTexture(file, meta);
	else if (ext == L"tif" || ext == L"tiff")
		ImportTexture(file, meta);
	else if (ext == L"jpg" || ext == L"jpeg")
		ImportTexture(file, meta);
	else if (ext == L"tga")
		ImportTexture(file, meta);
	else if (ext == L"dds")
		ImportTexture(file, meta);

	else if (ext == L"hlsl")
		CompileShader(file, meta, shaderIncludePaths);

	else if (ext == L"ttf")
		ImportFont(file, meta);
}
void LoadDirectory(jwstring dir, jvector<jwstring>* files, bool recursive) {
	jwstring d = dir + L"\\*";

	WIN32_FIND_DATAW ffd;
	HANDLE hFind = FindFirstFileW(d.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		wprintf(L"Failed to find directory %s\n", dir.c_str());
		return;
	}

	do {
		if (ffd.cFileName[0] == L'.') continue;

		jwstring c = dir + L"\\" + jwstring(ffd.cFileName);
		if (GetExtW(c) == L"meta") continue;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (recursive)
				LoadDirectory(c.c_str(), files, recursive);
		} else
			files->push_back(GetFullPathW(c));
	} while (FindNextFileW(hFind, &ffd) != 0);

	FindClose(hFind);
}

void UpdateControls(WPARAM wParam, RECT rect, InputState input) {
	for (int i = 0; i < controls.size(); i++)
		controls[i]->Update(wParam, rect, input);
}

void UILoadFile() {
	jwstring s = BrowseFile();
	if (!s.empty())
		fileTree->AddFile(s);
}
void UILoadFolder() {
	jwstring s = BrowseFolder();
	if (!s.empty())
		fileTree->AddFolder(s);
}
void UIClickFile(jwstring path) {
	jvector<AssetMetadata> assets;
	LoadFile(path, assets, jvector<jwstring>());
	if (!assets.empty()) {
		viewport->Show(assets[0]);
		properties->Show(assets[0]);
	}
}

LRESULT CALLBACK ViewportProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	RECT r;
	GetWindowRect(hwnd, &r);
	RECT c;
	GetClientRect(hwnd, &c);
	switch (message) {
	case WM_NCPAINT:
	{
		HDC hdc = GetDC(hwnd);
		FillRect(hdc, &r, Brushes::bgBrush);
		ReleaseDC(hwnd, hdc);
		break;
	}
	case WM_PAINT:
	{
		viewport->DoFrame();
		break;
	}

	case WM_SIZE:
	{
		viewport->Resize();
		break;
	}

	default:
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static bool leftButtonDown = false;
	static bool rightButtonDown = false;

	bool focused = GetFocus() == hwnd;

	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(hwnd, &cursor);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	switch (message) {
	case WM_CREATE:
	{
		controls.push_back(std::shared_ptr<Button>(new Button({ 0, 0, 0, 0 }, { 80, 20, 0, 0 }, L"Load File", UILoadFile)));
		controls.push_back(std::shared_ptr<Button>(new Button({ 80, 0, 0, 0 }, { 80, 20, 0, 0 }, L"Load Folder", UILoadFolder)));
		controls.push_back(std::shared_ptr<Label>(new Label({ 0, 20, 0, 0 }, { 100, 40, 0, 0 }, L"File Tree", Fonts::font18)));
		fileTree = std::shared_ptr<FileTree>(new FileTree({ 20, 60, 0, 0 }, { 350, -80, 0, 1 }, UIClickFile));
		controls.push_back(std::shared_ptr<Label>(new Label({ -370, 20, 1, 0 }, { 100, 40, 0, 0 }, L"Properties", Fonts::font18)));
		properties = std::shared_ptr<Properties>(new Properties({ -370, 60, 1, 0 }, { 350, -80, 0, .5f }, UILoadFile));
		controls.push_back(fileTree);
		controls.push_back(properties);

		LONG vpx = fileTree->CalcRect(clientRect).right + 5;
		LONG vpy = clientRect.top + 60;
		LONG vpw = properties->CalcRect(clientRect).left - fileTree->CalcRect(clientRect).right - 10;
		LONG vph = (clientRect.bottom - clientRect.top - 80) * 3 / 4;

		controls.push_back(std::shared_ptr<Label>(new Label({ (double)vpx, 20, 0, 0 }, { 100, 40, 0, 0 }, L"Viewport", Fonts::font18)));

		WNDCLASSEXW dlgClass = {};
		dlgClass.cbSize = sizeof(WNDCLASSEXW);
		dlgClass.style = CS_HREDRAW | CS_VREDRAW;
		dlgClass.lpfnWndProc = &ViewportProc;
		dlgClass.cbClsExtra = 0;
		dlgClass.cbWndExtra = 0;
		dlgClass.hInstance = HINST;
		dlgClass.hIcon = LoadIcon(HINST, NULL);
		dlgClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		dlgClass.hbrBackground = Brushes::bgBrush;
		dlgClass.lpszMenuName = NULL;
		dlgClass.lpszClassName = L"Jae Asset Packer Viewport";
		dlgClass.hIconSm = LoadIcon(HINST, NULL);
		HRESULT hr = RegisterClassExW(&dlgClass);
		assert(SUCCEEDED(hr));
		vphwnd = CreateWindowExW(NULL, L"Jae Asset Packer Viewport", L"", WS_CHILD, vpx, vpy, vpw, vph, hwnd, NULL, HINST, nullptr);
		assert(vphwnd);

		viewport = std::shared_ptr<Viewport>(new Viewport());
		viewport->Init(vphwnd);

		ShowWindow(vphwnd, SW_SHOW);

		break;
	}
	case WM_SIZE:
	{
		if (vphwnd) {
			LONG vpx = fileTree->CalcRect(clientRect).right + 5;
			LONG vpy = clientRect.top + 60;
			LONG vpw = properties->CalcRect(clientRect).left - fileTree->CalcRect(clientRect).right - 10;
			LONG vph = (clientRect.bottom - clientRect.top - 80) * 3 / 4;
			SetWindowPos(vphwnd, hwnd, vpx, vpy, vpw, vph, SWP_NOZORDER);
		}
		break;
	}

	case WM_MOUSEMOVE:
		UpdateControls(message, clientRect, { cursor, leftButtonDown, rightButtonDown, false, false, 1 });
		InvalidateRect(hwnd, NULL, false);
		break;
	case WM_LBUTTONDOWN:
		leftButtonDown = true;
		UpdateControls(message, clientRect, { cursor, leftButtonDown, rightButtonDown, true, false, 1 });
		InvalidateRect(hwnd, NULL, false);
		break;
	case WM_LBUTTONUP:
		leftButtonDown = false;
		UpdateControls(message, clientRect, { cursor, leftButtonDown, rightButtonDown, false, false, 1 });
		InvalidateRect(hwnd, NULL, false);
		break;
	case WM_RBUTTONDOWN:
		rightButtonDown = true;
		UpdateControls(message, clientRect, { cursor, leftButtonDown, rightButtonDown, false, true, 1 });
		InvalidateRect(hwnd, NULL, false);
		break;
	case WM_RBUTTONUP:
		rightButtonDown = false;
		UpdateControls(message, clientRect, { cursor, leftButtonDown, rightButtonDown, false, false, 1 });
		InvalidateRect(hwnd, NULL, false);
		break;
	case WM_LBUTTONDBLCLK:
		UpdateControls(message, clientRect, { cursor, leftButtonDown, rightButtonDown, true, false, 2 });
		InvalidateRect(hwnd, NULL, false);
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		HDC hdcm = CreateCompatibleDC(hdc);
		HBITMAP hbmp = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
		SelectObject(hdcm, hbmp);

		float dpiX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		float dpiY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;

		FillRect(hdcm, &clientRect, Brushes::bgBrush);
		SetBkMode(hdcm, TRANSPARENT);

		for (int i = 0; i < controls.size(); i++)
			controls[i]->Draw(hdcm, clientRect, true);

		BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcm, 0, 0, SRCCOPY);

		DeleteObject(hbmp);
		DeleteDC(hdcm);

		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
		JaeDestroy();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}

	return 0;
}
int uiMain() {
	//SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	// Check for DirectX Math library support.
	if (!DirectX::XMVerifyCPUSupport()) {
		MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	Fonts::Create();
	Brushes::Create();

	#pragma region register window class
	WNDCLASSEXW windowClass = {};

	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	windowClass.lpfnWndProc = &WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = HINST;
	windowClass.hIcon = LoadIcon(HINST, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = Brushes::bgBrush;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = L"Jae Asset Packer";
	windowClass.hIconSm = LoadIcon(HINST, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));

	HRESULT hr = RegisterClassExW(&windowClass);
	assert(SUCCEEDED(hr));
#pragma endregion

	#pragma region create window
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	RECT windowRect = { 0, 0, 1200L, 675L };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	// Center the window within the screen. Clamp to 0, 0 for the top-left corner.
	int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
	int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

	HWND hWnd = CreateWindowExW(
		NULL,
		L"Jae Asset Packer",
		L"Jae Asset Packer",
		WS_OVERLAPPEDWINDOW | CS_DBLCLKS,
		windowX,
		windowY,
		windowWidth,
		windowHeight,
		NULL,
		NULL,
		HINST,
		nullptr
	);

	assert(hWnd); // failed to create the window
#pragma endregion

	ShowWindow(hWnd, SW_SHOW);

	#pragma region message loop
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	#pragma endregion

	return 0;
}

int cmdMain(int argc, char** argv) {
	jvector<jwstring> files;
	jwstring output;
	jwstring input;
	bool compress = true;

	jvector<jwstring> includePaths;

	int mode = 0;
	for (int i = 1; i < argc; i++) {
		jwstring str = utf8toUtf16(jstring(argv[i]));
		if (argv[i][0] == L'-') {
			if (str == L"-f") {
				mode = 0;
			} else if (str == L"-d") {
				mode = 1;
			} else if (str == L"-dr") {
				mode = 4;
			} else if (str == L"-o") {
				mode = 2;
			} else if (str == L"-r") {
				mode = 3;
			} else if (str == L"-uc") {
				compress = false;
			} else if (str == L"-i") {
				mode = 5;
			}
		} else {
			switch (mode) {
			case 0:
				files.push_back(GetFullPathW(str));
				break;
			case 1:
				LoadDirectory(str, &files, false);
				break;
			case 2:
				output = str;
				break;
			case 3:
				input = GetFullPathW(str);
				break;
			case 4:
				LoadDirectory(str, &files, true);
				break;
			case 5:
				includePaths.push_back(GetFullPathW(str));
				break;
			}
		}
	}

	if (!input.empty()) {
		wprintf(L"Reading %s\n", input.c_str());
		jvector<Asset*> a = AssetFile::Read(input);
		for (int i = 0; i < a.size(); i++) {
			switch (a[i]->TypeId()) {
			case ASSET_TYPE_MESH:
				wprintf(L"%d vertices, %d tris\n", ((Mesh*)a[i])->VertexCount(), ((Mesh*)a[i])->IndexCount() / 3);
				break;
			case ASSET_TYPE_SHADER:
				wprintf(L"%s\n", a[i]->mName.c_str());
				break;
			case ASSET_TYPE_TEXTURE:
				wprintf(L"%dx%d\n", ((Texture*)a[i])->Width(), ((Texture*)a[i])->Height());
				break;
			}
			delete a[i];
		}
		wprintf(L"Successfully read %s\n", input.c_str());
		return 0;
	}

	if (output.empty()) wprintf(L"Please specify an output file [-o]");
	if (files.empty()) wprintf(L"Please specify input files [-f] or input directories [-d/-dr]");

	// print file names
	wprintf(L"Loading %d files\n", (int)files.size());
	for (int i = 0; i < files.size(); i++)
		wprintf(L"   %s\n", files[i].c_str());
	wprintf(L"\n");

	jvector<AssetMetadata> assetmeta;
	for (int i = 0; i < files.size(); i++)
		LoadFile(files[i], assetmeta, includePaths);

	wprintf(L"\n");

	jvector<Asset*> assets(assetmeta.size());
	for (int i = 0; i < assetmeta.size(); i++)
		assets.push_back(assetmeta[i].asset.get());

	AssetFile::Write(output.c_str(), assets, compress);

	return 0;
}

int main(int argc, char** argv) {
	Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_SINGLETHREADED);
	if (FAILED(initialize))
	{
		wprintf(L"Failed to initialize COM!\n");
		return -1;
	}

	if (argc == 1)
		// ui mode
		return uiMain();
	else
		// cmd line mode
		return cmdMain(argc, argv);
}