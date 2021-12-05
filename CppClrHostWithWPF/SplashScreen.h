#pragma once
#include "resource.h"
#include "framework.h"
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

class SplashScreen
{
public:
	SplashScreen(HINSTANCE hInstance, HWND hWnd);
	~SplashScreen();
	void Play();
	void OnTimer(UINT_PTR nIDEvent);
	void DrawCurrentFrame(HDC hdc);
private:
	Gdiplus::Bitmap* LoadImageFromResource(const wchar_t* resid, const wchar_t* restype);
	HINSTANCE m_hInstance;
	UINT_PTR m_timerId;
	HWND m_hWnd;
	Gdiplus::Image* m_pImage;
	GUID* m_pDimensionIDs;
	UINT m_FrameCount;
	Gdiplus::PropertyItem* m_pItem;
	UINT m_iCurrentFrame;
	Gdiplus::Bitmap* m_pMemBitmap;
};
