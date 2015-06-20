// fsclientwDlg.h : header file
//

#pragma once

#include <string>
#include <vector>
using std::vector;
using std::string;

#include "common.h"

#define WM_DOWNLOAD_FINISHED (WM_USER + 100)
#define MAX_THREADCOUNT 50

const int g_1G = 1024 * 1024 * 1024;
const int g_1M = 1024 * 1024;
const int g_1K = 1024;

enum status_idx
{
	idx_name = 0,
	idx_size,
	idx_dwsize,
	idx_ratio,
	idx_speed,
	idx_elapsed,
	idx_remains
};
//
int   str_endwith(const char* p, char c);
float size2float(int64 li, char& c);

struct file_info
{
	file_info()
	{
		reset();
	}
	int64   filesize;
	string  filename;
	string  basename;
	int     isFolder;
	time_t  lastmodify;
	void reset()
	{
		basename = "";
		filename = "";
		filesize = 0;
		isFolder = 0;
		lastmodify = 0;
	}
};

class DwManager;
struct pt_dwdata
{
	file_info* pfi;
	int        number;
	int64      offset;
	int64      length;
	int64      dwSize;
	DwManager* pdwm;
};

class DwManager
{
public:
	DwManager(const char* host, WORD port, int threadCount = 10);
	~DwManager();

private:
	int        m_threadCount;
	char       m_host[64];
	WORD       m_port;
	SOCKET     m_sockfd;
	TcpSocket* m_sock;

	pt_dwdata* m_ptDwData;

	CEdit*     m_logwin;
	CListCtrl* m_rptwin;
	CProgressCtrl* m_ppc;

	string     m_folder;
	string     m_dstFolder;
	string     m_tmpFolder;
	int        m_downloading;
	HANDLE*    m_pthdl;
	HANDLE     m_dwThread;
	HWND       m_msgHwnd;
	HANDLE     m_dwEvent;

	vector<file_info> m_dwfiles;

public:
	const char* host()
	{
		return m_host;
	}
	WORD port()
	{
		return m_port;
	}
	int downloading()
	{
		return m_downloading;
	}
	int threadsCount()
	{
		return m_threadCount;
	}
	void setDstFolder(const char* dstfolder)
	{
		m_dstFolder = dstfolder;
	}
	const char* dstFolder()
	{
		return m_dstFolder.c_str();
	}
	const char* tmpFolder()
	{
		return m_tmpFolder.c_str();
	}
	pt_dwdata* getDwInfo()
	{
		return m_ptDwData;
	}
	vector<file_info>& getDwFiles()
	{
		return m_dwfiles;
	}
	HANDLE* getThreadhandle()
	{
		return m_pthdl;
	}
	void setLogWin(CEdit* pe)
	{
		m_logwin = pe;
	}
	void setProCtrl(CProgressCtrl* ppc)
	{
		m_ppc = ppc;
	}
	CProgressCtrl* getProCtrl()
	{
		return m_ppc;
	}
	void setRptWin(CListCtrl* pr)
	{
		m_rptwin = pr;
	}
	CListCtrl* getRptWin()
	{
		return m_rptwin;
	}
	void setMsgHwnd(HWND hw)
	{
		m_msgHwnd = hw;
	}
	HWND getMsgHwnd()
	{
		return m_msgHwnd;
	}
public:
	void  setDwFile(const char* filename, int64 filesize = 0);
	int   connectServer(const char* host);
	int64 getFileSize(const char* filename);
	int   dir(vector<file_info>& vf, const char* folder = NULL);

	int   startDownload(vector<file_info>& vfiles, const char* folder = NULL);
	void  stopDownload()
	{
		m_downloading = 0;
	}
	void dwFinished();

	void waitDwEvent()
	{
		WaitForSingleObject(m_dwEvent, INFINITE);
	}
	FILE*   m_logfd;
	string  m_logContent;
	void    log(const char*  fmt, ...);
	void    report(const char*);
	void    combineFiles(const char*);
};
