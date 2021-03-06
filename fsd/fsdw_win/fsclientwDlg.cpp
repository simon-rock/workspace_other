// fsclientwDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fsclientw.h"
#include "fsclientwDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CfsclientwDlg::CfsclientwDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CfsclientwDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CfsclientwDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CfsclientwDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTCONN, &CfsclientwDlg::OnBnClickedButconn)
	ON_BN_CLICKED(IDOK, &CfsclientwDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CfsclientwDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTDW, &CfsclientwDlg::OnBnClickedButdw)
	ON_BN_CLICKED(IDC_BUTBROWSE, &CfsclientwDlg::OnBnClickedButbrowse)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FLIST, &CfsclientwDlg::OnLvnItemchangedFlist)
	ON_NOTIFY(NM_DBLCLK, IDC_FLIST, &CfsclientwDlg::OnNMDblclkFlist)
	ON_EN_CHANGE(IDC_EDIT_FILE, &CfsclientwDlg::OnEnChangeEditFile)
	ON_BN_CLICKED(IDC_BUTHELP, &CfsclientwDlg::OnBnClickedButhelp)
	ON_MESSAGE(WM_MWND_LBUTTONDOWN, &CfsclientwDlg::OnMyWndLeftUp) 
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


enum list_item
{ 
	IDX_FILENAME = 0,
	IDX_FILEINFO,
	IDX_FILESIZE,
	IDX_FILETIME,
	IDX_FULLNAME
};

BOOL CfsclientwDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//////////////////////////////////////////////////////////////////////////
	m_dwM = new DwManager("dev5", 18110, 10);
	m_dwWin.setDwManager(m_dwM);

	//m_dwM->setLogWin(pe);
	
	//////////////////////////////////////////////////////////////////////////
	m_font = new CFont;
	m_font->CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,FF_SWISS,(LPCTSTR)"Arial");
	CEdit* pe  = (CEdit *)GetDlgItem(IDC_EDIT_FILE);
	pe->SetFont(m_font, false); 
	pe->SetWindowText("/home");

	//////////////////////////////////////////////////////////////////////////
	CListCtrl * pl = (CListCtrl *)GetDlgItem(IDC_FLIST);
	pl->InsertColumn(IDX_FILENAME, "name", LVCFMT_LEFT,  200);
	pl->InsertColumn(IDX_FILEINFO, "info", LVCFMT_RIGHT, 80);
	pl->InsertColumn(IDX_FILESIZE, "size", LVCFMT_RIGHT, 120);
	pl->InsertColumn(IDX_FILETIME, "time", LVCFMT_RIGHT, 120);
	pl->InsertColumn(IDX_FULLNAME, "path", LVCFMT_LEFT,  200);	

	DWORD dwStyle = pl->GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	dwStyle |= LVS_EX_CHECKBOXES;
	pl->SetExtendedStyle(dwStyle);

	//////////////////////////////////////////////////////////////////////////
	CComboBox* pcb = (CComboBox*)GetDlgItem(IDC_COMBOSRV);
	//pcb->AddString("173.230.157.138");
	pcb->AddString("map1");
	pcb->AddString("dev5");
	pcb->AddString("tj-core4");
	pcb->SetCurSel(0);

	//////////////////////////////////////////////////////////////////////////
	CButton* pbut;
	pbut = (CButton*)GetDlgItem(IDC_BUTHELP);
	//m_icoHelp = (HICON)LoadImage(NULL, "d:/material/help.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	//pbut->SetIcon(m_icoHelp);

	pbut = (CButton*)GetDlgItem(IDC_BUTBROWSE);
	HICON m_icoFolder = (HICON)LoadImage(NULL, "icon/folder.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	pbut->SetIcon(m_icoFolder);

	pbut = (CButton*)GetDlgItem(IDC_BUTDW);
	HICON m_icoDown = (HICON)LoadImage(NULL, "icon/down.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	pbut->SetIcon(m_icoDown);

	//////////////////////////////////////////////////////////////////////////
	//m_logoWin.create(0x101, 610, 2, 56, 56, this);
	//m_logoWin.loadPng("icon/connected.png");
	//m_logoWin.loadPng("icon/connectlock.png");
	//m_logoWin.loadPng("icon/connecting.png");
	//m_logoWin.startTimer();
	//m_logoWin.ShowWindow(false);

	//
	RECT rect;
	GetClientRect(&rect);
	int x = rect.right - 36;

	m_buttonLog.create(0x102, x, 4, 32, 32, this);
	m_buttonLog.loadPng("icon/82.png");

	m_buttonHelp.create(0x103, x - 32, 4, 32, 32, this);
	m_buttonHelp.loadPng("icon/70.png");

	m_gifWin.create(0x101, 10, 2, 56, 56, this);
	m_gifWin.loadGif("icon/earth2.gif");
	m_gifWin.setText("x");
	//////////////////////////////////////////////////////////////////////////
	char buf[256];
	GetCurrentDirectory(sizeof(buf), buf);
	pe  = (CEdit *)GetDlgItem(IDC_EDIT_SAVETO);
	pe->SetFont(m_font, false); 
	pe->SetWindowText(buf);

	//////////////////////////////////////////////////////////////////////////
	enableButton("server",   true);
	enableButton("download", false);
	//m_logoWin.setImage(icon_bad);

	//////////////////////////////////////////////////////////////////////////
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CfsclientwDlg::OnPaint()
{
	CRect rect;
	GetClientRect(&rect);
	CPaintDC dc(this); // device context for painting

	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	//dc.FillSolidRect(rect.left, rect.top, rect.right, rect.bottom, 0xFFFFFF);
	COLORREF backcolor = ::GetSysColor(COLOR_BTNFACE);//dc.GetBkColor();
	dc.FillSolidRect(rect.left, rect.top, rect.right, rect.bottom, backcolor);
	//m_logoWin.Invalidate(true);
	m_buttonHelp.Invalidate(true);
	m_gifWin.Invalidate(true);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CfsclientwDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void InsertFileItem(CListCtrl* pl, int idx, file_info& fi, char* buf)
{
	int  n = pl->InsertItem(idx, buf);
	pl->SetItemText(n, IDX_FILENAME, fi.basename.c_str());

	if (fi.isFolder)
	{
		strcpy(buf, "<enter>");
	}
	else
	{
		char  c;
		float f = size2float(fi.filesize, c);
		sprintf(buf, "%.2f %c", f, c);
	}
	pl->SetItemText(n, IDX_FILEINFO, buf);

	sprintf(buf, "%llu", fi.filesize);
	pl->SetItemText(n, IDX_FILESIZE, buf);

	struct tm *tt = localtime(&fi.lastmodify);
	sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d",
		tt->tm_year + 1900, tt->tm_mon + 1, tt->tm_mday,
		tt->tm_hour, tt->tm_min, tt->tm_sec);
	pl->SetItemText(n, IDX_FILETIME, buf);
	pl->SetItemText(n, IDX_FULLNAME, fi.filename.c_str());
}

void CfsclientwDlg::dir(const char* folder)
{
	vector<file_info> vf;
	if (m_dwM->dir(vf, folder) < 0)
	{
		return;
	}

	CEdit * pe = (CEdit *)GetDlgItem(IDC_EDIT_FILE);
	pe->SetWindowText(folder);
	//enableButton("download", 0);

	CListCtrl * pl = (CListCtrl *)GetDlgItem(IDC_FLIST);
	pl->DeleteAllItems();
	char  buf[256];
	int64 total_size = 0;
	for (size_t i = 0; i < vf.size(); i++)
	{
		InsertFileItem(pl, (int)i, vf[i], buf);
		total_size += vf[i].filesize;
	}
	char  c;
	float f = size2float(total_size, c);
	sprintf(buf, " Files: %d,  size: %.2f %c", vf.size(), f, c);
	pe = (CEdit*)GetDlgItem(IDC_EDIT_STATUS);
	pe->SetWindowText(buf);
}

void CfsclientwDlg::OnBnClickedButconn()
{
	//m_logoWin.setImage(icon_doing);
	//m_logoWin.refresh();
	m_gifWin.setText("...");
	m_gifWin.refresh();
	//Sleep(100000);
	CComboBox * pcb = (CComboBox*)GetDlgItem(IDC_COMBOSRV);
	CString cstr;
	pcb->GetWindowText(cstr);
	if (cstr == "")
	{
		MessageBox("please select or input a server", "error");
		return;
	}
	if (m_dwM->connectServer(cstr.GetBuffer()) < 0)
	{
		MessageBox("failed to connect this server", "error");
		return;
	}

	dir("/home");
	enableButton("server",   false);
	enableButton("download", true);
	enableButton("connect",  false);
	//Sleep(3000);
	//m_logoWin.setImage(icon_ok);
	m_gifWin.setText("ok");
}

void CfsclientwDlg::OnBnClickedOk()
{
	//OnOK();
}

void CfsclientwDlg::OnBnClickedCancel()
{
	OnCancel();
}

void CfsclientwDlg::OnBnClickedButdw()
{
	CListCtrl* pl = (CListCtrl*)GetDlgItem(IDC_FLIST);
	file_info fi;
	vector<file_info> vfiles;
	char buf[MAX_PATH];
	for (int i = 0; i < pl->GetItemCount(); i++) 
	{
		if (pl->GetCheck(i) == 0)
		{
			continue;
		}
		pl->GetItemText(i, IDX_FILENAME, buf, sizeof(buf));
		if (strcmp(buf, "..") == 0)
		{
			continue;
		}
		fi.basename = buf;
		pl->GetItemText(i, IDX_FULLNAME, buf, sizeof(buf));
		fi.filename = buf;
		pl->GetItemText(i, IDX_FILEINFO, buf, sizeof(buf));
		fi.isFolder = !strcmp(buf, "<enter>");
		pl->GetItemText(i, IDX_FILESIZE, buf, sizeof(buf));
		fi.filesize = str_i64(buf);
		vfiles.push_back(fi);
	}
	if (vfiles.size())
	{
		m_dwWin.setDwFiles(vfiles);
		m_dwWin.DoModal();
		return;
	}
	CEdit* pe = (CEdit*)GetDlgItem(IDC_EDIT_FILE);
	char filename[256];
	pe->GetWindowText(filename, sizeof(filename));

	if (filename[0] == 0)
	{
		MessageBox("Please select or input a file name", "no file name");
		return;
	}
	if (filename[0] == '/' && filename[1] == 0)
	{
		MessageBox("Are your sure? you cannot download root directory", "wrong path");
		return;
	}

	//regular file
	if (!str_endwith(filename, '/'))
	{
		file_info fi;
		//nameConvert(filename, fi);
		fi.filesize = m_dwM->getFileSize(filename);
		if (fi.filesize < 0)
		{
			MessageBox("404 file not found", "error");
			return;
		}
		if (fi.filesize < 10)
		{
			MessageBox("I refuse to download file < 10 bytes", "error");
			return;
		}
		fi.filename = filename;
		char* pt = strrchr(filename, '/');
		if (pt)
		{
			fi.basename = pt + 1;
		}
		else
		{
			fi.basename = filename;
		}
		vfiles.push_back(fi);
		m_dwWin.setDwFiles(vfiles);
	}
	else //this is a folder, list files, give files and folder name to dw-win
	{
		size_t len = strlen(filename);
		filename[len - 1] = 0;
		m_dwM->dir(vfiles, filename);
		if (vfiles.size() == 0)
		{
			MessageBox("no files need to download", "error");
			return;
		}
		char* pt = strrchr(filename, '/');
		if (pt)
		{
			m_dwWin.setDwFiles(vfiles, pt + 1);
		}
		else
		{
			m_dwWin.setDwFiles(vfiles, filename);
		}

	}
	m_dwWin.DoModal();
}

void CfsclientwDlg::OnBnClickedButbrowse()
{
	char szPath[MAX_PATH];
	memset(szPath, 0, sizeof(szPath));   

	BROWSEINFO bi;   
	bi.hwndOwner = m_hWnd;   
	bi.pidlRoot = NULL;   
	bi.pszDisplayName = szPath;   
	bi.lpszTitle = "select a folder for downloading:";   
	bi.ulFlags = 0;   
	bi.lpfn = NULL;   
	bi.lParam = 0;   
	bi.iImage = 0;   
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);   

	if(lp && SHGetPathFromIDList(lp, szPath))   
	{
		CEdit* pe  = (CEdit *)GetDlgItem(IDC_EDIT_SAVETO);
		pe->SetWindowText(szPath);
		m_dwM->setDstFolder(szPath);
	}
}


void CfsclientwDlg::OnLvnItemchangedFlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	CListCtrl * pl = (CListCtrl *)GetDlgItem(IDC_FLIST);
	char buf[256];
	pl->GetItemText(pNMLV->iItem, IDX_FULLNAME, buf, sizeof(buf));
	CEdit * pe = (CEdit *)GetDlgItem(IDC_EDIT_FILE);
	pe->SetWindowText(buf);
}

void CfsclientwDlg::OnNMDblclkFlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;
	CListCtrl * pl = (CListCtrl *)GetDlgItem(IDC_FLIST);
	char buf[256];
	//pl->GetItemText(pNMLV->iItem, pNMLV->iSubItem, buf, sizeof(buf));
	//if ((pNMLV->iSubItem == IDX_FILEINFO) && (strcmp(buf, "<enter>") == 0))
	pl->GetItemText(pNMLV->iItem, IDX_FILEINFO, buf, sizeof(buf));
	if (strcmp(buf, "<enter>") == 0)
	{
		pl->GetItemText(pNMLV->iItem, IDX_FULLNAME, buf, sizeof(buf));
		CEdit * pe = (CEdit *)GetDlgItem(IDC_EDIT_FILE);
		//pe->SetWindowText("please wait a moment, connecting...");
		dir(buf);
		pe->SetWindowText(buf);
	}
}

void CfsclientwDlg::enableButton(const char* name, int enabled)
{
	CButton* pbut = NULL;
	if (strcmp(name, "connect") == 0)
	{
		pbut = (CButton*)GetDlgItem(IDC_BUTCONN);
		pbut->EnableWindow(enabled);
		return;
	}
	if (strcmp(name, "download") == 0)
	{
		pbut = (CButton*)GetDlgItem(IDC_BUTDW);
		pbut->EnableWindow(enabled);
		return;
	}
	if (strcmp(name, "server") == 0)
	{
		CComboBox* pcb = (CComboBox*)GetDlgItem(IDC_COMBOSRV);
		pcb->EnableWindow(enabled);
		return;
	}
}


void CfsclientwDlg::OnEnChangeEditFile()
{
// 	char buf[256];
// 	CEdit * pe = (CEdit *)GetDlgItem(IDC_EDIT_FILE);
// 	pe->GetWindowText(buf, sizeof(buf));
// 	m_dwM->setFolder(buf);
}

void CfsclientwDlg::OnBnClickedButhelp()
{
	MessageBox(
		"select or input a server to connect\r\n"
		"double click <enter> to enter a folder", 
		"help");
}

void CfsclientwDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	// m_dwM->stop();
	delete m_font;
	delete m_dwM;

	CDialog::OnClose();
}

void CfsclientwDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnTimer(nIDEvent);
}

LRESULT CfsclientwDlg::OnMyWndLeftUp(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case 0x101:
		MessageBox("this is server status", "server");
		break;
	case 0x102:
		OnBnClickedButhelp();
		break;
	case 0x103:
		m_gifWin.setEnabled(1);
		//Sleep(5000);
		break;
	default:
		break;
	}
	return 0;
}
