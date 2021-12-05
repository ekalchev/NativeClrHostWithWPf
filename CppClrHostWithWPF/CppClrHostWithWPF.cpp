// CppClrHostWithWPF.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "CppClrHostWithWPF.h"
#include <sstream>
#include "SplashScreen.h"
#include <versionhelpers.h>
#include <winuser.h>

#define WM_INITCLR (WM_USER + 0x0001)
#define SPLASH_WINDOW_WIDTH_DIPS 520
#define SPLASH_WINDOW_HEIGHT_DIPS 520

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hSplashScreenWnd;
SplashScreen* pSplashScreen;
Gdiplus::Color windowBorderColor = Gdiplus::Color::MakeARGB(255, 173, 173, 173);

// Forward declarations of functions included in this code module:
ATOM                SplashRegisterClass(HINSTANCE hInstance);
ATOM                DummyWindowRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProcDummyWindow(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcSplashScreen(HWND, UINT, WPARAM, LPARAM);

HRESULT RuntimeHost(PCWSTR pszVersion, PCWSTR pszAssemblyPath, PCWSTR pszClassName, PCWSTR pszStaticMethodName, void* pHostCallback);

void hideSplashScreen()
{
	if (!PostMessage(hSplashScreenWnd, WM_CLOSE, 0, 0))
	{
		//log
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// this might missing in Windows 7 - test it
	if (IsWindowsVersionOrGreater(HIBYTE(NTDDI_WIN10_RS2), LOBYTE(NTDDI_WIN10_RS2), 0)) {
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}
	else {
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
	}

	SplashRegisterClass(hInstance);
	DummyWindowRegisterClass(hInstance);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;

	// you can call here RuntimeHost but we want to let splashscreen window
	// to initialize and activate first and then to initialize clr
	if (!PostMessage(hSplashScreenWnd, WM_INITCLR, 0, 0))
	{
		//log
	}

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, 0, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete pSplashScreen;

	Gdiplus::GdiplusShutdown(gdiplusToken);

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM SplashRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcSplashScreen;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"SplashScreen";
	wcex.hIconSm = nullptr;

	return RegisterClassExW(&wcex);
}

ATOM DummyWindowRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcDummyWindow;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"DummyWindow";
	wcex.hIconSm = nullptr;

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	// old trick (a hidden owner window) to make the splash screen appear in the Alt+Tab list, but not in the taskbar. 
	// If you want the splash screen to also appear in the taskbar, you could drop the hidden owner window.

	HWND hwndOwner = CreateWindowEx(WS_EX_TOOLWINDOW, L"DummyWindow", nullptr, 0,
		0, 0, 0, 0, nullptr, nullptr, hInstance, NULL);

	/*hSplashScreenWnd = CreateWindowExW(WS_EX_LAYERED, L"SplashScreen", NULL, WS_POPUP | WS_VISIBLE,
		0, 0, 0, 0, hwndOwner, NULL, hInstance, NULL);*/

	HMONITOR monitor = MonitorFromWindow(hwndOwner, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	int monitor_width = info.rcMonitor.right - info.rcMonitor.left;
	int monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;

	// TODO - test with windows 7, alternative function GetDpiForMonitor
	double dpiScale = 96.0 / GetDpiForWindow(hwndOwner);
	int splash_window_dpiAware_width = SPLASH_WINDOW_WIDTH_DIPS / dpiScale;
	int splash_window_dpiAware_height = SPLASH_WINDOW_HEIGHT_DIPS / dpiScale;

	int x = (monitor_width - splash_window_dpiAware_width) / 2;
	int y = (monitor_height - splash_window_dpiAware_height) / 2;
	hSplashScreenWnd = CreateWindowEx(0, L"SplashScreen", L"SplashScreenTitle", WS_POPUP, x, y, splash_window_dpiAware_width, splash_window_dpiAware_height, hwndOwner, nullptr, hInstance, nullptr);

	if (!hSplashScreenWnd)
	{
		return FALSE;
	}

	ShowWindow(hSplashScreenWnd, nCmdShow);
	UpdateWindow(hSplashScreenWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProcSplashScreen(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITCLR:
		//RuntimeHost(L"v4.0.30319", L"ApplicationLib.dll", L"ApplicationLib.EntryPoint", L"Main", (void*)hideSplashScreen);
		break;
	case WM_CREATE:

		pSplashScreen = new SplashScreen(hInst, hWnd, windowBorderColor);
		pSplashScreen->Play();
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		pSplashScreen->DrawCurrentFrame(hdc);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_NCHITTEST: {
		// enable to move the window by clicking on the client area
		LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
		if (hit == HTCLIENT) hit = HTCAPTION;
		return hit;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		pSplashScreen->OnTimer(wParam);
		break;
	case WM_ERASEBKGND:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		pSplashScreen->DrawBackground(hdc);
		EndPaint(hWnd, &ps);
		return TRUE; // do not let windows to manage erase background because that causes flickering with gif animation
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK WndProcDummyWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
