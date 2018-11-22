// https://www.3dgep.com/learning-directx12-1/

#include "Util.hpp"

#include "AssetDatabase.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include "Profiler.hpp"
#include "Game.hpp"
#include "Shader.hpp"
#include "CommandQueue.hpp"
#include "Window.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Windowsx.h>

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

Game *g_Game;

// Window callback function.
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void ParseCommandLineArguments() {
	int argc;
	wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	for (size_t i = 0; i < argc; ++i) {
		//if (wcscmp(argv[i], L"-warp") == 0 || wcscmp(argv[i], L"--warp") == 0) {

		//}
	}

	// Free memory allocated by CommandLineToArgvW
	LocalFree(argv);
}

#define IsKeyDown(key) (GetAsyncKeyState(key) & 0x8000) != 0

HWND CreateWindow(const wchar_t* windowClassName, HINSTANCE hInst, const wchar_t* windowTitle, uint32_t width, uint32_t height) {
	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

	RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	// Center the window within the screen. Clamp to 0, 0 for the top-left corner.
	int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
	int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

	HWND hWnd = ::CreateWindowExW(
		NULL,
		windowClassName,
		windowTitle,
		WS_OVERLAPPEDWINDOW,
		windowX,
		windowY,
		windowWidth,
		windowHeight,
		NULL,
		NULL,
		hInst,
		nullptr
	);

	assert(hWnd && "Failed to create window");

	return hWnd;
}

void DoFrame() {
	static std::chrono::high_resolution_clock clock;
	static auto start = clock.now();
	static auto t0 = clock.now();
	static int frameCounter;
	static double elapsedSeconds;

	Profiler::FrameStart();

	auto t1 = clock.now();
	double delta = (t1 - t0).count() * 1e-9;
	t0 = t1;

	Profiler::BeginSample("Update");
	g_Game->Update((t1 - start).count() * 1e-9, delta);
	Profiler::EndSample();

	Profiler::BeginSample("Render");
	Graphics::Render(g_Game);
	Profiler::EndSample();

	// measure fps
	frameCounter++;
	elapsedSeconds += delta;
	if (elapsedSeconds > 1.0) {
		g_Game->m_fps = frameCounter / elapsedSeconds;
		frameCounter = 0;
		elapsedSeconds = 0.0;
	}

	Input::FrameEnd();
	Profiler::FrameEnd();
}
void ReadInput(MSG msg) {
	UINT dwSize;
	GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	LPBYTE lpb = new BYTE[dwSize];
	if (lpb == 0) return;
	if (GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		OutputDebugString("Incorrect GetRawInputData size\n");
	RAWINPUT* raw = (RAWINPUT*)lpb;

	if (raw->header.dwType == RIM_TYPEMOUSE) {
		int x = raw->data.mouse.lLastX;
		int y = raw->data.mouse.lLastY;
		Input::OnMouseMoveEvent(x, y);
		
		if (Input::m_LockMouse) {
			RECT rect = Graphics::GetWindow()->GetRect();
			SetCursorPos((rect.right + rect.left) / 2, (rect.bottom + rect.top) / 2);
		}
		
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN)
			Input::OnMousePressEvent(0, true);
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)
			Input::OnMousePressEvent(0, false);
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN)
			Input::OnMousePressEvent(1, true);
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP)
			Input::OnMousePressEvent(1, false);
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN)
			Input::OnMousePressEvent(2, true);
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP)
			Input::OnMousePressEvent(2, false);
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
			Input::OnMousePressEvent(3, true);
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
			Input::OnMousePressEvent(3, false);
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
			Input::OnMousePressEvent(4, true);
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
			Input::OnMousePressEvent(4, false);

		if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
			Input::OnMouseWheelEvent((short)raw->data.mouse.usButtonData / (float)WHEEL_DELTA);
	}
	if (raw->header.dwType == RIM_TYPEKEYBOARD) {
		bool pressed = (raw->data.keyboard.Flags & RI_KEY_BREAK) == 0;
		Input::OnKeyDownEvent((KeyCode::Key)raw->data.keyboard.VKey, pressed);
	}

	delete[] lpb;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (!Graphics::IsInitialized()) return ::DefWindowProcW(hwnd, message, wParam, lParam);

	switch (message) {
		case WM_PAINT:
			if (Graphics::FrameReady())
				DoFrame();
			break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			break;

		// The default window procedure will play a system notification sound 
		// when pressing the Alt+Enter keyboard combination if this message is 
		// not handled.
		case WM_SYSCHAR:
			break;
		case WM_SIZE:
		{
			auto window = Graphics::GetWindow();
			RECT clientRect = {};
			::GetClientRect(window->GetHandle(), &clientRect);

			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;

			window->Resize();
			g_Game->OnResize();
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProcW(hwnd, message, wParam, lParam);
	}

	return 0;
}

void RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName) {
	// Register a window class for creating our render window with.
	WNDCLASSEXW windowClass = {};

	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = &WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInst;
	windowClass.hIcon = ::LoadIcon(hInst, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));
	windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = ::LoadIcon(hInst, NULL); //  MAKEINTRESOURCE(APPLICATION_ICON));

	static HRESULT hr = ::RegisterClassExW(&windowClass);
	assert(SUCCEEDED(hr));
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// Window class name. Used for registering / creating the window.
	const wchar_t* windowClassName = L"Jae3d";
	ParseCommandLineArguments();

	// Check for DirectX Math library support.
	if (!DirectX::XMVerifyCPUSupport()) {
		MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	RegisterWindowClass(hInstance, windowClassName);

	HWND hWnd = CreateWindow(windowClassName, hInstance, L"Jae3d dx12", 1280, 720);

	// register raw input devices
	RAWINPUTDEVICE rID[2];
	// Mouse
	rID[0].usUsagePage = 0x01;
	rID[0].usUsage = 0x02;
	rID[0].dwFlags = 0;
	rID[0].hwndTarget = NULL;// Graphics::m_hWnd;
	// Keyboard
	rID[1].usUsagePage = 0x01;
	rID[1].usUsage = 0x06;
	rID[1].dwFlags = 0;
	rID[1].hwndTarget = NULL;// Graphics::m_hWnd;
	if (RegisterRawInputDevices(rID, 2, sizeof(RAWINPUTDEVICE)) == FALSE)
		OutputDebugString("Failed to register raw input device(s)\n");

	Graphics::Initialize(hWnd);
	AssetDatabase::LoadAssets("common.asset");
	g_Game = new Game();
	g_Game->Initialize(Graphics::GetCommandQueue()->GetCommandList());

	ShowWindow(hWnd, SW_SHOW);

	// Main loop
	bool run = true;
	MSG msg = {};
	while (run) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				run = false;
				break;
			}
			if (msg.message == WM_INPUT)
				ReadInput(msg);
		}
	}

	Graphics::Destroy();

	delete g_Game;

	return 0;
}