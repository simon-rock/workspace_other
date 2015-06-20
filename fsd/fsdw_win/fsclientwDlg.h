// fsclientwDlg.h : header file
//

#pragma once

#include <string>
#include <vector>
using std::vector;
using std::string;

#include "dwmanager.h"
#include "dwdialog.h"
#include "mWnd.h"

// CfsclientwDlg dialog
class CfsclientwDlg : public CDialog
{
// Construction
public:
	CfsclientwDlg(CWnd* pParent = NULL);	

// Dialog Data
	enum { IDD = IDD_FSCLIENTW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	//
	DwManager* m_dwM;
	CDwDlg     m_dwWin;
	CFont*     m_font;

	CmWnd      m_logoWin;
	CmWnd      m_buttonLog;
	CmWnd      m_buttonHelp;
	CmWnd      m_buttonDown;
	CmWnd      m_buttonBrowse;
	CmWnd      m_gifWin;

// Implementation
protected:
	HICON m_hIcon;

	void dir(const char* folder = NULL);
	void enableButton(const char* name, int enabled);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButconn();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButdw();
	afx_msg void OnBnClickedButbrowse();
	afx_msg void OnLvnItemchangedFlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkFlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditFile();
	afx_msg void OnBnClickedButhelp();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	afx_msg LRESULT OnMyWndLeftUp(WPARAM wParam, LPARAM lParam); 
};
