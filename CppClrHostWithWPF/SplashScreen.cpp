
#include "SplashScreen.h"


SplashScreen::SplashScreen(HINSTANCE hInstance, HWND hWnd, Gdiplus::Color windowBorderColor)
{
	m_hWnd = hWnd;
	m_iCurrentFrame = 0;
	m_hInstance = hInstance;
	m_pLoadingAnimation = LoadImageFromResource(MAKEINTRESOURCE(IDI_LOADING_GIF1_0x), L"GIF");
	m_timerId = 10001;

	RECT rect;
	GetWindowRect(hWnd, &rect);
	m_windowSize = new Gdiplus::Size(rect.right - rect.left, rect.bottom - rect.top);

	m_windowBorderPen = new Gdiplus::Pen(windowBorderColor, 1.0f);
	//First of all we should get the number of frame dimensions
	//Images considered by GDI+ as:
	//frames[animation_frame_index][how_many_animation];
	UINT count = m_pLoadingAnimation->GetFrameDimensionsCount();

	//Now we should get the identifiers for the frame dimensions 
	m_pDimensionIDs = new GUID[count];
	m_pLoadingAnimation->GetFrameDimensionsList(m_pDimensionIDs, count);

	//For gif image , we only care about animation set#0
	WCHAR strGuid[39];
	int result = StringFromGUID2(m_pDimensionIDs[0], strGuid, 39);

	m_FrameCount = m_pLoadingAnimation->GetFrameCount(&m_pDimensionIDs[0]);

	//PropertyTagFrameDelay is a pre-defined identifier 
	//to present frame-delays by GDI+
	UINT TotalBuffer = m_pLoadingAnimation->GetPropertyItemSize(PropertyTagFrameDelay);
	m_pItem = (Gdiplus::PropertyItem*)malloc(TotalBuffer);
	m_pLoadingAnimation->GetPropertyItem(PropertyTagFrameDelay, TotalBuffer, m_pItem);
	m_pBackgroundBitmap = new Gdiplus::Bitmap(m_windowSize->Width, m_windowSize->Height);

	Gdiplus::Graphics* pMemGraphics = Gdiplus::Graphics::FromImage(m_pBackgroundBitmap);
	Gdiplus::Bitmap* backgroundImage = LoadImageFromResource(MAKEINTRESOURCE(IDI_BACKGROUND), L"PNG");
	pMemGraphics->DrawImage(backgroundImage, 0, 0, m_windowSize->Width, m_windowSize->Height);
	delete backgroundImage;
	delete pMemGraphics;
}

SplashScreen::~SplashScreen()
{
	if (m_pDimensionIDs) delete[] m_pDimensionIDs;
	if (m_pItem) free(m_pItem);
	if (m_pBackgroundBitmap) delete m_pBackgroundBitmap;
	if (m_pLoadingAnimation) delete m_pLoadingAnimation;
	if (m_windowSize) delete m_windowSize;
	if (m_windowBorderPen) delete m_windowBorderPen;
	if (m_pCachedBackgroundBitmap) delete m_pCachedBackgroundBitmap;
}

//To start play
void SplashScreen::Play()
{
	//Set Current Frame at #0
	m_iCurrentFrame = 0;
	GUID Guid = Gdiplus::FrameDimensionTime;
	m_pLoadingAnimation->SelectActiveFrame(&Guid, m_iCurrentFrame);

	//Use Timer
	//NOTE HERE: frame-delay values should be multiply by 10
	SetTimer(m_hWnd, m_timerId, ((UINT*)m_pItem[0].value)[m_iCurrentFrame] * 10, NULL);

	//Move to the next frame
	++m_iCurrentFrame;
	InvalidateRect(m_hWnd, nullptr, TRUE);
}

//Using timer
void SplashScreen::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_timerId)
	{
		//Because there will be a new delay value
		KillTimer(m_hWnd, m_timerId);

		//Change Active frame
		GUID Guid = Gdiplus::FrameDimensionTime;
		m_pLoadingAnimation->SelectActiveFrame(&Guid, m_iCurrentFrame);

		//New timer
		SetTimer(m_hWnd, m_timerId, ((UINT*)m_pItem[0].value)[m_iCurrentFrame] * 10, NULL);

		//Again move to the next
		m_iCurrentFrame = (++m_iCurrentFrame) % m_FrameCount;
		InvalidateRect(m_hWnd, nullptr, FALSE);
	}
}

//Present current frame
void SplashScreen::DrawCurrentFrame(HDC hdc)
{
	Gdiplus::Graphics* pGraphics = new Gdiplus::Graphics(hdc);
	DrawCurrentFrame(pGraphics);
	delete pGraphics;
}

void SplashScreen::DrawCurrentFrame(Gdiplus::Graphics* pGraphics)
{
	pGraphics->DrawImage(m_pLoadingAnimation, 1, 450, m_windowSize->Width - 2, m_pLoadingAnimation->GetHeight());
}

void SplashScreen::DrawBackground(HDC hdc)
{
	Gdiplus::Rect rect = Gdiplus::Rect(0, 0, m_windowSize->Width - 1, m_windowSize->Height - 1);
	Gdiplus::Graphics* pGraphics = new Gdiplus::Graphics(hdc);

	if (m_pCachedBackgroundBitmap == nullptr)
	{
		m_pCachedBackgroundBitmap = new Gdiplus::CachedBitmap(m_pBackgroundBitmap, pGraphics);
	}

	pGraphics->DrawCachedBitmap(m_pCachedBackgroundBitmap, 0, 0);
	pGraphics->DrawRectangle(m_windowBorderPen, rect);
	DrawCurrentFrame(pGraphics);

	delete pGraphics;
}

Gdiplus::Bitmap* SplashScreen::LoadImageFromResource(const wchar_t* resid, const wchar_t* restype)
{
	IStream* pStream = nullptr;
	Gdiplus::Bitmap* pBmp = nullptr;
	HGLOBAL hGlobal = nullptr;

	HRSRC hrsrc = FindResourceW(m_hInstance, resid, restype);     // get the handle to the resource
	if (hrsrc)
	{
		DWORD dwResourceSize = SizeofResource(m_hInstance, hrsrc);
		if (dwResourceSize > 0)
		{
			HGLOBAL hGlobalResource = LoadResource(m_hInstance, hrsrc); // load it
			if (hGlobalResource)
			{
				void* imagebytes = LockResource(hGlobalResource); // get a pointer to the file bytes

				// copy image bytes into a real hglobal memory handle
				hGlobal = ::GlobalAlloc(GHND, dwResourceSize);
				if (hGlobal)
				{
					void* pBuffer = ::GlobalLock(hGlobal);
					if (pBuffer)
					{
						memcpy(pBuffer, imagebytes, dwResourceSize);
						HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
						if (SUCCEEDED(hr))
						{
							// pStream now owns the global handle and will invoke GlobalFree on release
							hGlobal = nullptr;
							pBmp = new Gdiplus::Bitmap(pStream);
						}
					}
				}
			}
		}
	}

	if (pStream)
	{
		pStream->Release();
		pStream = nullptr;
	}

	if (hGlobal)
	{
		GlobalFree(hGlobal);
		hGlobal = nullptr;
	}

	return pBmp;
}