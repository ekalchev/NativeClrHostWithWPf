// CppClrHostWithWPF.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "CppClrHostWithWPF.h"
#include <sstream>
#include "SplashScreen.h"

#define WM_INITCLR (WM_USER + 0x0001)

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hSplashScreenWnd;
SplashScreen* pSplashScreen;

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

	HWND hwndOwner = CreateWindowExW(WS_EX_TOOLWINDOW, L"DummyWindow", nullptr, 0,
		0, 0, 0, 0, nullptr, nullptr, hInstance, NULL);

	/*hSplashScreenWnd = CreateWindowExW(WS_EX_LAYERED, L"SplashScreen", NULL, WS_POPUP | WS_VISIBLE,
		0, 0, 0, 0, hwndOwner, NULL, hInstance, NULL);*/

	hSplashScreenWnd = CreateWindowW(L"SplashScreen", L"SplashScreenTitle", WS_OVERLAPPED,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hwndOwner, nullptr, hInstance, nullptr);

	SetWindowLong(hSplashScreenWnd, GWL_STYLE, 0);

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
		pSplashScreen = new SplashScreen(hInst, hWnd);
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
		LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
		if (hit == HTCLIENT) hit = HTCAPTION; // move window by clicking its client area
		return hit;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		pSplashScreen->OnTimer(wParam);
		break;
	case WM_ERASEBKGND:
		return TRUE; // do not let windows to manage erase background because that causes flickering with gif animation
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
