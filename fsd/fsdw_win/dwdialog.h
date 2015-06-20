#pragma once

/////////////////////////////////////////////////////////////////////////////
#include "resource.h"
#include "dwmanager.h"

class CDwDlg : public CDialog
{
public:
	CDwDlg();

	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_DIALOG_DW };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButDwStart();
	afx_msg LRESULT OnDownLoadFinished(WPARAM wParam, LPARAM lParam); 
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedDwcancel();

public:

	DwManager* m_pdwm;
	string m_folder;
	vector<file_info> m_dwFiles;

	void enableButton(int, int, int);
	void setDwFiles(vector<file_info>&, const char* folder = "");
	void setDwManager(DwManager* pdwm)
	{
		m_pdwm = pdwm;
	}
};

/////////////////////////////////////////////////////////////////////////////
