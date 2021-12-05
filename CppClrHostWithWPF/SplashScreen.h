#pragma once
#include "resource.h"
#include "framework.h"
#include <objidl.h>
#include <gdiplus.h>
#include <gdipluspen.h>
#pragma comment(lib,"gdiplus.lib")

class SplashScreen
{
public:
	SplashScreen(HINSTANCE hInstance, HWND hWnd, Gdiplus::Color windowBorderColor);
	~SplashScreen();
	void Play();
	void OnTimer(UINT_PTR nIDEvent);
	void DrawCurrentFrame(HDC hdc);
	void DrawBackground(HDC hdc);
private:
	void DrawCurrentFrame(Gdiplus::Graphics* pHdc);
	Gdiplus::Bitmap* LoadImageFromResource(const wchar_t* resid, const wchar_t* restype);
	Gdiplus::Pen* m_windowBorderPen;
	Gdiplus::Size *m_windowSize;
	HINSTANCE m_hInstance;
	UINT_PTR m_timerId;
	HWND m_hWnd;
	Gdiplus::Image* m_pLoadingAnimation;
	GUID* m_pDimensionIDs;
	UINT m_FrameCount;
	Gdiplus::PropertyItem* m_pItem;
	UINT m_iCurrentFrame;
	Gdiplus::Bitmap* m_pBackgroundBitmap;
	Gdiplus::CachedBitmap* m_pCachedBackgroundBitmap;
};
