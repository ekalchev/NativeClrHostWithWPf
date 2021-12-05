
#include "SplashScreen.h"


SplashScreen::SplashScreen(HINSTANCE hInstance, HWND hWnd)
{
	m_hWnd = hWnd;
	m_iCurrentFrame = 0;
	m_hInstance = hInstance;
	m_pImage = LoadImageFromResource(MAKEINTRESOURCE(IDI_LOADING_GIF1_0x), L"GIF");
	m_timerId = 10001;

	//First of all we should get the number of frame dimensions
	//Images considered by GDI+ as:
	//frames[animation_frame_index][how_many_animation];
	UINT count = m_pImage->GetFrameDimensionsCount();

	//Now we should get the identifiers for the frame dimensions 
	m_pDimensionIDs = new GUID[count];
	m_pImage->GetFrameDimensionsList(m_pDimensionIDs, count);

	//For gif image , we only care about animation set#0
	WCHAR strGuid[39];
	int result = StringFromGUID2(m_pDimensionIDs[0], strGuid, 39);

	m_FrameCount = m_pImage->GetFrameCount(&m_pDimensionIDs[0]);

	//PropertyTagFrameDelay is a pre-defined identifier 
	//to present frame-delays by GDI+
	UINT TotalBuffer = m_pImage->GetPropertyItemSize(PropertyTagFrameDelay);
	m_pItem = (Gdiplus::PropertyItem*)malloc(TotalBuffer);
	m_pImage->GetPropertyItem(PropertyTagFrameDelay, TotalBuffer, m_pItem);
	m_pMemBitmap = new Gdiplus::Bitmap(m_pImage->GetWidth(), m_pImage->GetHeight());
}

SplashScreen::~SplashScreen()
{
	if (m_pDimensionIDs)
	{
		delete[] m_pDimensionIDs;
	}

	if (m_pItem)
	{
		free(m_pItem);
	}

	if (m_pMemBitmap)
	{
		delete m_pMemBitmap;
	}

	if (m_pImage)
	{
		delete m_pImage;
	}
}

//To start play
void SplashScreen::Play()
{
	//Set Current Frame at #0
	m_iCurrentFrame = 0;
	GUID Guid = Gdiplus::FrameDimensionTime;
	m_pImage->SelectActiveFrame(&Guid, m_iCurrentFrame);

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
		m_pImage->SelectActiveFrame(&Guid, m_iCurrentFrame);

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
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);

	Gdiplus::Graphics* pMemGraphics = Gdiplus::Graphics::FromImage(m_pMemBitmap);
	Gdiplus::Status status = pMemGraphics->DrawImage(m_pImage, 0, 0, m_pImage->GetWidth(), m_pImage->GetHeight());

	if (status != Gdiplus::Status::Ok)
	{
		//error
	}

	Gdiplus::Graphics* pHdc = new Gdiplus::Graphics(hdc);
	status = pHdc->DrawImage(m_pMemBitmap, 0, 0);

	if (status != Gdiplus::Status::Ok)
	{
		//error
	}

	delete pHdc;
	delete pMemGraphics;
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