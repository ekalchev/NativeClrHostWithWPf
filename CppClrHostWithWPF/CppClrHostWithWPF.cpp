// CppClrHostWithWPF.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "CppClrHostWithWPF.h"
#include <sstream>

#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

#define WM_INITCLR (WM_USER + 0x0001)

using namespace Gdiplus;

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hSplashScreenWnd;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT RuntimeHost(PCWSTR pszVersion, PCWSTR pszAssemblyPath, PCWSTR pszClassName, PCWSTR pszStaticMethodName, void* pHostCallback);

void hideSplashScreen()
{
    if (!PostMessage(hSplashScreenWnd, WM_CLOSE, 0, 0))
    {
        //log
    }
}

HBITMAP loadImageWithGdiPlus(LPCTSTR pszPngPath)
{
    wchar_t error[4096];

    Image *pImage = Image::FromFile(pszPngPath);
    
    if (pImage->GetLastStatus() != Status::Ok)
    {
        wprintf(L"%s failed to load through GDI+", error);
    }

    UINT count = pImage->GetFrameDimensionsCount();

    //Now we should get the identifiers for the frame dimensions 
    auto m_pDimensionIDs = new GUID[count];
    pImage->GetFrameDimensionsList(m_pDimensionIDs, count);

    //For gif image , we only care about animation set#0
    WCHAR strGuid[39];
    StringFromGUID2(m_pDimensionIDs[0], strGuid, 39);
    auto m_FrameCount = pImage->GetFrameCount(&m_pDimensionIDs[0]);

    //PropertyTagFrameDelay is a pre-defined identifier 
    //to present frame-delays by GDI+
    UINT TotalBuffer = pImage->GetPropertyItemSize(PropertyTagFrameDelay);
    auto m_pItem = (PropertyItem*)malloc(TotalBuffer);
    pImage->GetPropertyItem(PropertyTagFrameDelay, TotalBuffer, m_pItem);

    int width = pImage->GetWidth();
    int height = pImage->GetHeight();

    BITMAPINFO bmi;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biClrImportant = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biSizeImage = 0; //calc later
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    BYTE* pBmp = NULL;
    HBITMAP hbm = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pBmp, NULL, 0);
    HDC hdc = CreateCompatibleDC(NULL);
    HGDIOBJ hobj = SelectObject(hdc, hbm);

    Graphics graphics(hdc);
    graphics.DrawImage(pImage, 0, 0);

    SelectObject(hdc, hobj);
    DeleteDC(hdc);
    return hbm;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
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

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, 0, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GdiplusShutdown(gdiplusToken);

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"SplashScreen";
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

   HWND hwndOwner = CreateWindowExW(WS_EX_TOOLWINDOW, L"SplashScreen", nullptr, 0,
       0, 0, 0, 0, nullptr, nullptr, hInstance, NULL);

   //hSplashScreenWnd = CreateWindowExW(WS_EX_LAYERED, szWindowClass, NULL, WS_POPUP | WS_VISIBLE,
   //    0, 0, 0, 0, hwndOwner, NULL, hInstance, NULL);

   hSplashScreenWnd = CreateWindowW(L"SplashScreen", L"SplashScreenTitle", WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hwndOwner, nullptr, hInstance, nullptr);

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITCLR:
        RuntimeHost(L"v4.0.30319", L"ApplicationLib.dll", L"ApplicationLib.EntryPoint", L"Main", (void*)hideSplashScreen);
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
