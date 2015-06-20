#pragma once


//////////////////////////////////////////////////////////////////////////
// CmWnd window
//////////////////////////////////////////////////////////////////////////
#include <atlimage.h>
#include <gdiplusimaging.h>
//#include <gdiplus.h>
#include <string>
using std::string;

#define WM_MWND_LBUTTONDOWN (WM_USER + 101)
#define IMAGE_MAX 8

enum icon_status
{
	icon_ok,
	icon_bad,
	icon_doing
};

//typedef  void (*fpButtonFunc)(void);
//typedef  void (*fpButtonFunc)(void);
// class CmGif
// {
// public:
// 	CmGif();
// 	~CmGif();
// 
// 	Gdiplus::Image* m_gif;
// 	int*    m_pause;
// 	HANDLE  m_thread;
// 	int     m_running;
// 
// 	int load(const char* p);
// };

class CmWnd : public CWnd
{
public:
	CmWnd();
	virtual ~CmWnd();

protected:
	//track mouse
	BOOL     m_mouseIn;
	int      m_id;
	char     m_winEnabled;
	string   m_text;

	//image
	CImage   m_image;
	HBITMAP  m_hbitmap[IMAGE_MAX];
	//int    m_transColor[IMAGE_MAX];

	//gif
	Gdiplus::Image* m_gif;
	int*     m_gifPause;
	HANDLE   m_gifThread;
	char     m_gifRunning;

	//common
	int      m_imgWidth;
	int      m_imgHeight;
	char     m_imgMax;
	char     m_imgCur;

public:
	//window
	int create(int id, RECT& rect, CWnd* parent)
	{
		m_id = id;
		return Create(NULL, NULL, WS_VISIBLE, rect, parent, id, NULL);
	}
	int create(int id, int x, int y, int dx, int dy, CWnd* parent)
	{
		RECT rect;
		rect.left = x; rect.right = rect.left + dx;
		rect.top  = y; rect.bottom= rect.top  + dy;
		m_id = id;
		return Create(NULL, NULL, WS_VISIBLE, rect, parent, id, NULL);
	}
	void setText(const char* p)
	{
		m_text = p;
	}
	void setEnabled(int enabled)
	{
		m_winEnabled = enabled;
	}
	//for image
	int   loadPng(const char* png, int idx = -1);
	int   loadGif(const char* gif);
	int   loadBmp(const char* bmp);

	int   gifRunning()
	{
		return m_gifRunning;
	}
	void  updateGif();
	void  setImage(int cur);
	void  refresh();

	int   runPng();

protected:
	//{{AFX_MSG(CmWnd)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseEnter(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////
