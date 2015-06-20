// mWnd.cpp : implementation file
//

#include "stdafx.h"
#include "mWnd.h"

//////////////////////////////////////////////////////////////////////////
// CmWnd

CmWnd::CmWnd()
{
	//image
	m_imgWidth  = 0;
	m_imgHeight = 0;
	m_imgMax    = 0;
	m_imgCur    = 0;

	//win
	m_mouseIn    = false;
	m_winEnabled = 1;
	m_id         = 0;

	//png
	for (int i = 0; i < IMAGE_MAX; i++)
	{
		m_hbitmap[i] = NULL;
	}

	//gif
	m_gif        = NULL;
	m_gifThread  = INVALID_HANDLE_VALUE;
	m_gifRunning = 0;
	m_gifPause   = NULL;
}

CmWnd::~CmWnd()
{
	//KillTimer(1);
	//KillTimer(2);
	for (int i = 0; i < IMAGE_MAX; i++)
	{
		if (m_hbitmap[i] != NULL)
		{
			DeleteObject(m_hbitmap[i]);
		}
	}
	if (m_gif)
	{
		if (m_gifThread != INVALID_HANDLE_VALUE)
		{
			m_gifRunning = 0;
			::WaitForSingleObject(m_gifThread, INFINITE);
			::CloseHandle(m_gifThread);
		}
		if (m_gifPause)
		{
			delete[] m_gifPause;
		}
		delete m_gif;
	}
}

BEGIN_MESSAGE_MAP(CmWnd, CWnd)
	//{{AFX_MSG_MAP(CmWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, &CmWnd::OnMouseLeave) 
	ON_MESSAGE(WM_MOUSEHOVER, &CmWnd::OnMouseEnter) 
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CmWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	return 0;
}

int CmWnd::loadBmp(const char* bmp)
{
	HBITMAP bi = (HBITMAP)LoadImage(NULL, bmp, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (bi == NULL)
	{
		return -1;
	}
	m_hbitmap[0] = bi;
	return 0;
}

unsigned __stdcall draw_gif(void* args)
{
	CmWnd* pwm = (CmWnd*)args;
	while (pwm->gifRunning())
	{
		pwm->updateGif();
	}
	return 0;
}

int CmWnd::loadGif(const char* gif)
{
	m_imgCur  = 0;
	m_imgMax  = 0;

	wchar_t pw[256];
	mbstowcs(pw, gif, 256);

	m_gif = Gdiplus::Image::FromFile(pw);
	if (m_gif == NULL)
	{
		MessageBox("cannot load gif", gif);
		return -1;
	}

	m_imgWidth  = m_gif->GetWidth();
	m_imgHeight = m_gif->GetHeight();

	// get frame count
	/* I think one is enough
	int count = m_gif->GetFrameDimensionsCount();
	GUID* pGUIDs = new GUID[count];
	m_gif->GetFrameDimensionsList(&pGUIDs, count);
	m_imgMax = m_gif->GetFrameCount(&pGUIDs[0]);
	delete[] pGUIDs;
	*/
	GUID oneUID;
	m_gif->GetFrameDimensionsList(&oneUID, 1);
	m_imgMax = m_gif->GetFrameCount(&oneUID);

	int size = m_gif->GetPropertyItemSize(PropertyTagFrameDelay);
	if (size > 0)
	{
		char* tmp = new char[size];
		Gdiplus::PropertyItem* pItem = (Gdiplus::PropertyItem*)tmp;
		m_gif->GetPropertyItem(PropertyTagFrameDelay, size, pItem);

		m_gifPause = new int[m_imgMax];
		for (int i = 0; i < m_imgMax; i++)
		{
			m_gifPause[i] = ((long*)pItem->value)[i] * 10;
		}
		delete[] tmp;
	}

	m_gifRunning = 0;
	if (m_imgMax > 1)
	{
		m_gifRunning = 1;
		m_gifThread = (HANDLE*)_beginthreadex(NULL, 0, &draw_gif, (void*)this, 0, NULL);
	}
	return 0;
}

void CmWnd::updateGif()
{
	if (m_winEnabled == 0)// || !m_mouseIn)
	{
		Sleep(100);
		return;
	}
	GUID Guid = Gdiplus::FrameDimensionTime;
	m_gif->SelectActiveFrame(&Guid, m_imgCur);
	m_imgCur = (++m_imgCur) % m_imgMax;
	Invalidate(true);
	Sleep(m_gifPause[m_imgCur]);
}

//
int CmWnd::loadPng(const char* png, int idx)
{
	if (m_imgMax >= IMAGE_MAX)
	{
		MessageBox(png, "open too many files");
		return -1;
	}
	if (m_image.Load(png))
	{
		MessageBox(png, "cannot load file");
		return -1;
	}

	m_imgWidth  = m_image.GetWidth();
	m_imgHeight = m_image.GetHeight();
	unsigned char* pucColor;
	for(int i = 0; i < m_image.GetWidth(); i++)  
	{  
		for(int j = 0; j < m_image.GetHeight(); j++)  
		{  
			pucColor = (unsigned char*)m_image.GetPixelAddress(i, j);  
			if (pucColor[3] == 255)
			{
				//continue;
			}
			pucColor[0] = BYTE(MulDiv(pucColor[0], pucColor[3], 255));  
			pucColor[1] = BYTE(MulDiv(pucColor[1], pucColor[3], 255));  
			pucColor[2] = BYTE(MulDiv(pucColor[2], pucColor[3], 255));  
		}  
	}  
	if (idx == -1 || IMAGE_MAX)
	{
		//m_transColor[m_imageTotal] = m_image.GetTransparentColor();
		m_hbitmap[m_imgMax] = m_image.Detach();
	}
	else
	{
		//m_transColor[idx] = m_image.GetTransparentColor();
		m_hbitmap[idx] = m_image.Detach();
	}

	++m_imgMax;
	return 0;
}

void CmWnd::OnPaint() 
{
	CPaintDC dc(this); 
	CDC memdc;
	memdc.CreateCompatibleDC(&dc);

	RECT rect;
	GetClientRect(&rect);
	int x = rect.left;
	int y = rect.top;
	int dx= rect.right - rect.left;
	int dy= rect.bottom - rect.top;

	CBitmap bit;
	bit.CreateBitmap(dx, dy, 1, 32, NULL);
	CBitmap* old = (CBitmap*)memdc.SelectObject(&bit);

	COLORREF backcolor = ::GetSysColor(COLOR_BTNFACE);
	if (m_mouseIn)
	{
		memdc.FillSolidRect(x, y, dx, dy, backcolor + 0x080808);
		memdc.Draw3dRect(x, y, dx, dy, 0x707070, 0x808080);
	}
	else
	{
		memdc.FillSolidRect(x, y, dx, dy, backcolor);
	}

	if (m_hbitmap[m_imgCur])
	{
		//we supposed every png is in same size
		CDC pngdc;
		pngdc.CreateCompatibleDC(&dc);
		HGDIOBJ org = pngdc.SelectObject(m_hbitmap[m_imgCur]);

		int mx = x;
		int my = y;
		int mdx = dx;
		int mdy = dy;
		if (!m_mouseIn)
		{
			mx += 4; my += 4;
			mdx -= 8; mdy -= 8;
		}
		BLENDFUNCTION bf;  
		bf.BlendOp = AC_SRC_OVER;  
		bf.BlendFlags = 0;  
		bf.SourceConstantAlpha = 255;  
		bf.AlphaFormat = AC_SRC_ALPHA; 
		AlphaBlend(
			memdc.m_hDC, mx, my, mdx, mdy, 
			pngdc.m_hDC, 0, 0, m_imgWidth, m_imgHeight, 
			bf);

		pngdc.SelectObject(org);
		pngdc.DeleteDC();
	}

	if (m_gif)
	{
		CDC gifdc;
		gifdc.CreateCompatibleDC(&memdc);
		CBitmap bit;
		bit.CreateBitmap(m_imgWidth, m_imgHeight, 1, 32, NULL);
		CBitmap* old = (CBitmap*)gifdc.SelectObject(&bit);
		gifdc.FillSolidRect(0, 0, m_imgWidth, m_imgWidth, backcolor);

		Gdiplus::Graphics gh(gifdc.m_hDC); 

		int mx = (m_imgWidth  > dx) ? x : (dx - m_imgWidth)  / 2;
		int my = (m_imgHeight > dy) ? y : (dy - m_imgHeight) / 2;
		gh.DrawImage(m_gif, 0, 0, m_imgWidth, m_imgHeight);

		memdc.StretchBlt(mx, my, dx, dy, &gifdc, 0, 0, m_imgWidth, m_imgHeight, SRCCOPY);
		gifdc.SelectObject(old);
		bit.DeleteObject();
		gifdc.DeleteDC();
	}

	if (m_text.length())
	{
		CSize sz = memdc.GetTextExtent(m_text.c_str(), m_text.length());
		int mx = (sz.cx > dx) ? x : (dx - sz.cx) / 2;
		int my = (sz.cy > dy) ? y : (dy - sz.cy) / 2;
		memdc.SetTextColor(0x00FF00);
		memdc.SetBkMode(0);
		memdc.TextOut(mx, my, m_text.c_str());
	}
	dc.BitBlt(x, y, dx, dy, &memdc, 0, 0, SRCCOPY);
	memdc.SelectObject(old);
	bit.DeleteObject();
	memdc.DeleteDC();
}

void CmWnd::setImage(int cur)
{
	m_imgCur = cur;
	Invalidate(TRUE);
}

void CmWnd::refresh()
{
	OnPaint();
}

int CmWnd::runPng()
{
	SetTimer(1, 1500, NULL);
	return 0;
}

void CmWnd::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case 1:
		if (m_imgMax)
		{
			m_imgCur = (++m_imgCur) % m_imgMax;
			Invalidate(true);
		}
		break;
	case 2:
		{
			POINT mouseXY;
			GetCursorPos(&mouseXY);
			ScreenToClient(&mouseXY);
			RECT rect;
			GetClientRect(&rect);
			if (!PtInRect(&rect, mouseXY))
			{
				KillTimer(2);
				OnMouseLeave(0, 0);
			}
		}
		break;
	default:
		break;
	}

}

void CmWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_mouseIn)
	{
		m_mouseIn = true;
		OnMouseEnter(0, 0);
		SetTimer(2, 100, NULL);
	}
}

LRESULT CmWnd::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_mouseIn = false;
	Invalidate(true);
	return 0;
}

LRESULT CmWnd::OnMouseEnter(WPARAM wParam, LPARAM lParam)
{
	Invalidate(true);
	return 0;
}

void CmWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	::PostMessage(GetParent()->m_hWnd, WM_MWND_LBUTTONDOWN, m_id, 0);
}
