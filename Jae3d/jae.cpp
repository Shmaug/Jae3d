#include "Util.hpp"

#include "AssetDatabase.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include "Profiler.hpp"
#include "IJaeGame.hpp"
#include "Shader.hpp"
#include "CommandQueue.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windowsx.h>
#include "Window.hpp"
#include <shellapi.h>

#include "jae.hpp"

using namespace std;

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

IJaeGame* g_Game;

// Window callback function.
LRESULT CALLBACK JaeWndProc(HWND, UINT, WPARAM, LPARAM);

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
		
		if (Input::mLockMouse) {
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

LRESULT CALLBACK JaeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (!Graphics::IsInitialized()) return ::DefWindowProcW(hwnd, message, wParam, lParam);

	switch (message) {
		case WM_PAINT:
			if (g_Game && Graphics::FrameReady())
				g_Game->DoFrame();
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
			if (g_Game) g_Game->OnResize();
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
	windowClass.lpfnWndProc = &JaeWndProc;
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

HWND JaeCreateWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR title, int width, int height) {
	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// Check for DirectX Math library support.
	if (!DirectX::XMVerifyCPUSupport()) {
		MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	RegisterWindowClass(hInstance, className);
	HWND hWnd = CreateWindow(className, hInstance, title, width, height);

	// register raw input devices
	RAWINPUTDEVICE rID[2];
	// Mouse
	rID[0].usUsagePage = 0x01;
	rID[0].usUsage = 0x02;
	rID[0].dwFlags = 0;
	rID[0].hwndTarget = NULL;
	// Keyboard
	rID[1].usUsagePage = 0x01;
	rID[1].usUsage = 0x06;
	rID[1].dwFlags = 0;
	rID[1].hwndTarget = NULL;
	if (RegisterRawInputDevices(rID, 2, sizeof(RAWINPUTDEVICE)) == FALSE)
		OutputDebugString("Failed to register raw input device(s)\n");

	Graphics::Initialize(hWnd);
	ShowWindow(hWnd, SW_SHOW);

	return hWnd;
}

void JaeMsgLoop(IJaeGame* game) {
	g_Game = game;
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
}

void JaeDestroy() {
	AssetDatabase::UnloadAssets();
	Graphics::Destroy();
}