// https://www.3dgep.com/learning-directx12-1/

#include "Util.h"

#include "Graphics.h"
#include "Input.h"
#include "Profiler.h"
#include "Game.h"
#include "Shader.h"
#include "CommandQueue.h"

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

Game *g_game;
HANDLE g_mutex;

// Window callback function.
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void ParseCommandLineArguments() {
	int argc;
	wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

	for (size_t i = 0; i < argc; ++i) {
		//if (::wcscmp(argv[i], L"-warp") == 0 || ::wcscmp(argv[i], L"--warp") == 0) {

		//}
	}

	// Free memory allocated by CommandLineToArgvW
	::LocalFree(argv);
}

int DecodeMouseButton(UINT messageID) {
	int mouseButton = 0;

	switch (messageID) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
			mouseButton = 0;
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
			mouseButton = 1;
			break;

		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
			mouseButton = 2;
			break;

		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDBLCLK:
			mouseButton = 3;
			break;
	}

	return mouseButton;
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (!Graphics::IsInitialized()) return ::DefWindowProcW(hwnd, message, wParam, lParam);

	switch (message) {
		case WM_PAINT:
			break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			Input::OnKeyDownEvent((KeyCode::Key)wParam, true);
		}
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			Input::OnKeyDownEvent((KeyCode::Key)wParam, false);
		}
			break;
		case WM_MOUSEMOVE:
		{
			Input::OnMouseMoveEvent(((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
		}
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			Input::OnMousePressEvent(DecodeMouseButton(message), true);
		}
			break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			Input::OnMousePressEvent(DecodeMouseButton(message), false);
		}
			break;
		case WM_MOUSEWHEEL:
		{
			Input::OnMouseWheelEvent(((int)(short)HIWORD(wParam)) / (float)WHEEL_DELTA);
		}
			break;

		// The default window procedure will play a system notification sound 
		// when pressing the Alt+Enter keyboard combination if this message is 
		// not handled.
		case WM_SYSCHAR:
			break;
		case WM_SIZE:
		{
			RECT clientRect = {};
			::GetClientRect(Graphics::m_hWnd, &clientRect);

			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;

			Graphics::Resize(width, height);
			g_game->OnResize();
		}
		break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			break;
		default:
			return ::DefWindowProcW(hwnd, message, wParam, lParam);
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

	g_game = new Game();
	g_mutex = CreateMutex(NULL, FALSE, NULL);

	HWND hWnd = CreateWindow(windowClassName, hInstance, L"Jae3d dx12", Graphics::m_ClientWidth, Graphics::m_ClientHeight);

	// Initialize the global window rect variable.
	::GetWindowRect(hWnd, &Graphics::m_WindowRect);

	Graphics::Initialize(hWnd);
	g_game->Initialize(Graphics::GetCommandQueue()->GetCommandList());
	Graphics::OnRender = std::bind(&Game::Render, g_game, std::placeholders::_1);
	ShaderLibrary::LoadShaders();

	::ShowWindow(hWnd, SW_SHOW);

	Graphics::StartRenderLoop(g_mutex);

	static std::chrono::high_resolution_clock clock;
	static auto start = clock.now();
	static auto t0 = clock.now();
	static int frameCounter;
	static double elapsedSeconds;

	// Main loop
	MSG msg = {};
	while (msg.message != WM_QUIT) {
		// Begin the frame; wait for the render thread to release the mutex
		Profiler::FrameStart();

		// Wait for the mutex, ensures render thread isn't doing anything while the main thread updates (and vice versa)
		Profiler::BeginSample("Wait for render thread");
		if (WaitForSingleObject(g_mutex, INFINITE) & WAIT_ABANDONED) break;
		Profiler::EndSample();

		Profiler::BeginSample("Windows events");
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		Profiler::EndSample();

		Profiler::BeginSample("Update");
		auto t1 = clock.now();
		double delta = (t1 - t0).count() * 1e-9;
		t0 = t1;
		g_game->Update((t1 - start).count() * 1e-9, delta);
		Profiler::EndSample();

		Input::FrameEnd();
		ReleaseMutex(g_mutex);

		// measure fps
		frameCounter++;
		elapsedSeconds += delta;
		if (elapsedSeconds > 1.0) {
			g_game->m_fps = frameCounter / elapsedSeconds;
			Graphics::m_fps = Graphics::GetAndResetFPS() / elapsedSeconds;
			
			frameCounter = 0;
			elapsedSeconds = 0.0;
		}
		Profiler::FrameEnd();
	}

	Graphics::Destroy();

	delete(g_game);

	return 0;
}