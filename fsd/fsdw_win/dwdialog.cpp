// mWnd.cpp : implementation file
//

#include "stdafx.h"
#include "dwdialog.h"

/////////////////////////////////////////////////////////////////////////////


CDwDlg::CDwDlg() : CDialog(CDwDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CDwDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDwDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUT_DW_START, &CDwDlg::OnBnClickedButDwStart)
	ON_MESSAGE(WM_DOWNLOAD_FINISHED, &CDwDlg::OnDownLoadFinished) 
	ON_BN_CLICKED(IDC_BUT_DW_STOP, &CDwDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_DWCANCEL, &CDwDlg::OnBnClickedDwcancel)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////

BOOL CDwDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
		
	CListCtrl* pl = (CListCtrl*)GetDlgItem(IDC_LIST_DWFILES);
	pl->InsertColumn(idx_name,    "file name",      LVCFMT_LEFT,  160);
	pl->InsertColumn(idx_size,    "total",          LVCFMT_RIGHT, 80);
	pl->InsertColumn(idx_dwsize,  "finished",       LVCFMT_RIGHT, 80);
	pl->InsertColumn(idx_ratio,   "ratio",          LVCFMT_RIGHT, 60);
	pl->InsertColumn(idx_speed,   "average",        LVCFMT_RIGHT, 80);
	pl->InsertColumn(idx_elapsed, "time elapsed",   LVCFMT_RIGHT, 80);
	pl->InsertColumn(idx_remains, "time remains",   LVCFMT_RIGHT, 80);

	m_pdwm->setRptWin(pl);
	m_pdwm->setMsgHwnd(this->m_hWnd);

	CProgressCtrl* ppc = (CProgressCtrl*)GetDlgItem(IDC_PROGRESSDW);
	ppc->SetRange(0, 100);
	ppc->SetPos(0);
	m_pdwm->setProCtrl(ppc);

	float  f;
	char   c;
	char   buf[256];
	UINT64 total_size = 0;
	size_t idx = 0;
	while (idx < m_dwFiles.size())
	{
		pl->InsertItem((int)idx, m_dwFiles[idx].basename.c_str());
		f = size2float(m_dwFiles[idx].filesize, c);
		sprintf(buf, "%.2f %c", f, c);
		pl->SetItemText((int)idx, idx_size, buf);
		if (m_dwFiles[idx].isFolder || m_dwFiles[idx].filesize < 10)
		{
			++idx;
			continue;
		}
		total_size += m_dwFiles[idx].filesize;
		++idx;
	}

	CEdit* pe = (CEdit*)GetDlgItem(IDC_EDIT_DW_MSG);
	f = size2float(total_size, c);
	sprintf(buf, "%d files, %.2f %c", idx, f, c);
	pe->SetWindowText(buf);

	enableButton(true, false, true);
	return TRUE;
}

void CDwDlg::enableButton(int forStart, int forStop, int forCancel)
{
	CButton* pbut;
	pbut = (CButton*)GetDlgItem(IDC_BUT_DW_START);
	pbut->EnableWindow(forStart);
	pbut = (CButton*)GetDlgItem(IDC_BUT_DW_STOP);
	pbut->EnableWindow(forStop);
	pbut = (CButton*)GetDlgItem(IDC_DWCANCEL);
	pbut->EnableWindow(forCancel);
}

void CDwDlg::setDwFiles(vector<file_info>& vf, const char* folder)
{
	m_dwFiles.clear();
	m_folder = folder;
	for (size_t i = 0; i < vf.size(); i++)
	{
		if (vf[i].basename == "../")
		{
			continue;
		}
		m_dwFiles.push_back(vf[i]);
	}
}

void CDwDlg::OnBnClickedButDwStart()
{
	//CProgressCtrl* ppc = (CProgressCtrl*)GetDlgItem(IDC_PROGRESSDW);
	//ppc->SetPos(0);
	enableButton(false, true, false);

	if (m_pdwm->startDownload(m_dwFiles, m_folder.c_str()) < 0)
	{
		MessageBox("cannot open file", "error");
		enableButton(true, false, true);
	}
}

LRESULT CDwDlg::OnDownLoadFinished(WPARAM wParam, LPARAM lParam)
{
	m_pdwm->stopDownload();
	m_pdwm->dwFinished();

	CButton* pbut = (CButton*)GetDlgItem(IDC_DWCANCEL);
	pbut->SetWindowText("Finished");
	enableButton(false, false, true);
	return 0;
}

void CDwDlg::OnBnClickedStop()
{
	if (!m_pdwm->downloading())
	{
		return;
	}
	m_pdwm->stopDownload();
	enableButton(true, false, true);
}


void CDwDlg::OnBnClickedDwcancel()
{
	CDialog::OnCancel();
}

