// fsclientwDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fsclientw.h"
#include "dwmanager.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
int str_endwith(const char* p, char c)
{
	size_t len = strlen(p);
	if (len == 0)
	{
		return 0;
	}
	return (p[len - 1] == c);
}

float size2float(int64 li, char& c)
{
	if (li > g_1G)
	{
		c = 'g';
		int64 ltmp = li >> 20;
		float f = (float)ltmp;
		return f / 1024;
	}
	if (li > g_1M)
	{
		c = 'm';
		return (float)li / g_1M;
	}
	c = 'k';
	return (float)li / g_1K;
}

//////////////////////////////////////////////////////////////////////////
DwManager::DwManager(const char* host, WORD port, int threadCount)
{
	m_threadCount = threadCount;
	strcpy(m_host , host);
	m_port      = port;
	m_dstFolder = ".";

	m_pthdl    = new HANDLE[m_threadCount];
	m_dwThread = INVALID_HANDLE_VALUE;
	m_ptDwData = new pt_dwdata[threadCount];
	m_logfd    = NULL;
	m_dwEvent  = CreateEvent(NULL, FALSE, FALSE, "dw");

	m_downloading = 0;
	socket_init(2, 2);
	m_sock = new TcpSocket(30, 2048);
}

DwManager::~DwManager()
{
	delete[] m_pthdl;
	delete[] m_ptDwData;
	delete m_sock;
	CloseHandle(m_dwEvent);
	socket_cleanup();
}

void nameConvert(const char* sfile, file_info& fi)
{
	char filename[256];
	// \\map1\data2\xxx\xx.gz, we need to skip \\map1	
	if (strncmp(sfile, "\\\\map1", 6) == 0)
	{
		strcpy(filename, sfile + 6);
	}
	else
	{
		strcpy(filename, sfile);
	}
	int len = 0;
	int last_slash = 0;
	while (filename[len])
	{
		if (filename[len] == '\\')
		{
			filename[len] = '/';
		}
		if (filename[len] == '/')
		{
			last_slash = len;
		}
		len++;
	}
	fi.filename = filename;
	if (last_slash)
	{
		fi.basename = filename + last_slash + 1;
	}
	else
	{
		fi.basename = filename;
	}
}

int DwManager::connectServer(const char* host)
{
	m_sock->close();
	char buf[1024];
	strcpy(m_host, host);

	log("trying to connect [%s]:[%d]", host, m_port);
	if (m_sock->connect(m_host, m_port) < 0)
	{
		log("!!Error: cannot connect to server");
		return -1;
	}

	int n = m_sock->readline(buf, sizeof(buf));
	if (n <= 0)
	{
		log("!!Error: read msg failed from server");
		return -1;
	}

	buf[n] = 0;
	log("srv: <=%s", buf);

	m_folder = "/home";
	return 0;
}

int64 DwManager::getFileSize(const char* filename)
{
	char buf[1024];
	sprintf(buf,"size %s\n", filename);
	log("=>%s", buf);
	m_sock->send(buf);

	int n = m_sock->readline(buf, sizeof(buf));
	if (n <= 0)
	{
		log("!!Error: connection error");
		return -1;
	}
	log("<=%s", buf);
	if (strncmp(buf, "+OK", 3) != 0)
	{
		log("!!Error: wrong response");	
		return -1;
	}

	int64 total = str_i64(buf + 4);
	if (total <= 0)
	{
		return -1;
	}

	log("get size ok: %llu", total);
	return total;
}

// file.cpp:0:3280:12334455
int parse_fileinfo(char* buf, file_info& fo)
{
	char* pt;
	#define FIND_NEXT \
		pt = strchr(buf, ':');\
		if (pt == NULL)\
		{\
			return -1;\
		}\
		*pt = 0;\
		pt++;

	FIND_NEXT;
	fo.basename = buf;
	fo.isFolder = *pt - '0';
	if (fo.isFolder && !str_endwith(buf, '/'))
	{
		fo.basename.append("/");
	}

	buf = pt + 2; //skip :
	FIND_NEXT
	fo.filesize   = str_i64(buf);
	fo.lastmodify = atoi(pt);
	return 0;
}

//  /a,   /,  a, ...
void make_fullname(const char* pfolder, const char* file, string& sfolder)
{
	char buf[256];
	int  i = 0;
	while (pfolder[i])
	{
		buf[i] = pfolder[i];
		++i;
	}
	buf[i] = 0;
	if (i && (buf[i - 1] == '/'))
	{
		buf[--i] = 0;
	}
	if (strcmp(file, "../") == 0)
	{
		char* pt = strrchr(buf, '/');
		if ((pt == NULL) || (pt == buf))
		{
			sfolder = "/";
			return;
		}
		*pt = 0;
		sfolder = buf;
	}
	else
	{
		sfolder = buf;
		sfolder.append("/");
		sfolder.append(file);
	}
}

int DwManager::dir(vector<file_info>& vf, const char* folder)
{
	char buf[1024];
	// dir folder
	sprintf(buf,"dir %s\n", folder);
	log("=>%s", buf);
	m_sock->send(buf);

	//srv: +OK filecount\r\n
	//filename:isFolder:filesize:lastmodify
	m_sock->readline(buf, sizeof(buf));
	if (buf[0] != '+')
	{
		return -1;
	}
	int filen = atoi(buf + 4);
	
	file_info fo;
	while (filen--)
	{
		m_sock->readline(buf, sizeof(buf));
		log("<=%s", buf);
		if (parse_fileinfo(buf, fo) < 0)
		{
			break;
		}
		make_fullname(folder, fo.basename.c_str(), fo.filename);
		vf.push_back(fo);
	}
	return 0;
}

void DwManager::log(const char *fmt, ...)
{
	if (m_logfd == NULL)
	{
		m_logfd = fopen("fslog.txt", "a");
	}
	va_list	ap;
	va_start(ap, fmt);
	vfprintf(m_logfd, fmt, ap);
	va_end (ap);
	fprintf(m_logfd, "\n");
	fflush(m_logfd);
	return;

	char buf[256];
	//va_list	ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end (ap);
	strcat(buf, "\r\n");
	m_logContent += buf;
	if (m_logContent.length() > 10000)
	{
		m_logContent = m_logContent.c_str() + 5000;
	}
	m_logwin->SetWindowText(m_logContent.c_str());
	m_logwin->SetSel((int)m_logContent.length(), (int)m_logContent.length(), FALSE);
}

void DwManager::report(const char* buf)
{
	m_rptwin->SetWindowText(buf);
}

void DwManager::combineFiles(const char* basename)
{
	char localname[256];

	char name[256];
	sprintf(name, "%s/%s", m_tmpFolder.c_str(), basename);
	log("combine files to: %s", name);
	FILE* fp = fopen(name, "wb");
	if (fp == NULL)
	{
		log("cannot open [%s]");
		return;
	}

	char buf[10240];
	struct _stat64 fst;

	for (int i = 0; i < m_threadCount; i++)
	{
		sprintf(localname, "%s/tmp/%s_%d", m_tmpFolder.c_str(), basename, i);
		FILE* fpt = fopen(localname, "rb");
		if (fpt == NULL)
		{
			log("cannot open %s", localname);
			break;
		}
		_stat64(localname, &fst);
		int64 dataLeft = fst.st_size;
		while (dataLeft)
		{	
			int n = sizeof(buf) > dataLeft ? dataLeft : sizeof(buf);
			fread(buf, 1, n, fpt);
			fwrite(buf, 1, n, fp);
			dataLeft -= n;
		}
		fclose(fpt);
		DeleteFile(localname);
	}
	fclose(fp);	
	log("move file to [%s]", name);
}

unsigned __stdcall pt_worker(void* args)
{
	pt_dwdata* pdp  = (struct pt_dwdata*)args;
	DwManager* pdwm = pdp->pdwm;

	char   localname[256];
	sprintf(localname, "%s/tmp/%s_%d", 
		pdwm->tmpFolder(), pdp->pfi->basename.c_str(), pdp->number);

	//check if file is already exists
	int64 total = 0;
	FILE* fp = fopen(localname, "rb");
	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		total = ftell(fp);
		fclose(fp);
		fp = fopen(localname, "ab");
	}
	else
	{
		fp = fopen(localname, "wb");
	}

	pdp->length -= total;
	pdp->offset += total;
	pdp->dwSize =  total;

	int bufsize = 2048;
	char* sockbuf = new char[bufsize];
	TcpSocket dwsock;
	while (pdwm->downloading() && pdp->length > 0)
	{
		dwsock.close();
		if (dwsock.connect(pdwm->host(), pdwm->port()) < 0)
		{
			pdwm->log("thread %d connect to server failed, will retry");
			Sleep(5000);
			continue;
		}

		dwsock.readline(sockbuf, bufsize);
		pdwm->log("thread: %d connect server ok", pdp->number);

		sprintf(sockbuf, "get %s %llu %llu\n",
			pdp->pfi->filename.c_str(), pdp->offset, pdp->length);
		pdwm->log(sockbuf);
		dwsock.send(sockbuf, (int)strlen(sockbuf));

		int64 len;
		int   nread;
		while (pdwm->downloading() && pdp->length > 0)
		{	
			len = bufsize > pdp->length ? pdp->length : bufsize;
			nread = dwsock.recv(sockbuf, len);
			if (nread <= 0)
			{
				break;
			}
			fwrite(sockbuf, 1, nread, fp);
			pdp->length -= nread;
			pdp->offset += nread;
			pdp->dwSize += nread;
		}
	}
	fclose(fp);
	delete[] sockbuf;
	pdwm->log("task finished. thread: %d", pdp->number);
	return 0;
}

struct status_data
{
	int   idx;
	int64 filesize;
	int64 currsize;
	float dwspeed;
	int   timeElapsed;
	int   timeRemains;
};

unsigned __stdcall pt_dw(void* args)
{
	DwManager* pdwm = (DwManager*)args;

	vector<file_info>& vfi = pdwm->getDwFiles();
	CListCtrl* plist = pdwm->getRptWin();

	int64 total_size = 0;
	int64 total_dwsize = 0;
	char  buf[1024];
	float f;
	char  c;
	size_t idx = 0;
	while (idx < vfi.size())
	{
		if (vfi[idx].isFolder || vfi[idx].filesize < 10)
		{
			++idx;
			continue;
		}
		total_size += vfi[idx].filesize;
		++idx;
	}

	int timesleep = 1;
	status_data sdt;
	idx = 0;
	while (idx < vfi.size() && pdwm->downloading())
	{
		if (vfi[idx].isFolder || vfi[idx].filesize < 10)
		{
			pdwm->log("skip dir or files < 10: [%s]", vfi[idx].filename.c_str());
			++idx;
			continue;
		}

		memset(&sdt, 0, sizeof(sdt));
		sdt.filesize = vfi[idx].filesize;
		//log("start to download [%s]", filename);
		//calculate data size for each thread
		int   threadCount = pdwm->threadsCount();
		int64 pt_len = (sdt.filesize + threadCount) / threadCount;
		pt_dwdata* pwd = pdwm->getDwInfo();
		for (int i = 0; i < threadCount; i++)
		{
			pwd[i].pfi    = &vfi[idx];
			pwd[i].number = i;
			pwd[i].offset = pt_len * i;
			pwd[i].length = pt_len;
			pwd[i].dwSize = 0;
			pwd[i].pdwm   = pdwm;
		}
		pwd[threadCount - 1].length = sdt.filesize % pt_len;

		//create threads
		HANDLE* pthdl = pdwm->getThreadhandle();
		for (int i = 0; i < threadCount; i++)
		{
			pthdl[i] = (HANDLE*)_beginthreadex(NULL, 0, &pt_worker, (void*)&pwd[i], 0, NULL);
		}

		int etimeHour, etimeMinute, etimeSeconds;
		int rtimeHour, rtimeMinute, rtimeSeconds;
		int64 lastsize = 0;
		int64 diffsize = 0;
		int64 tmp64;
		while (pdwm->downloading() && (sdt.currsize < sdt.filesize))
		{
			Sleep(timesleep * 1000);
			sdt.timeElapsed += timesleep;
			sdt.currsize = 0;
			for (int i = 0; i < threadCount; i++)
			{
				sdt.currsize += pwd[i].dwSize;
			}
			tmp64 = (total_dwsize + sdt.currsize) / 1024;
			f = (float)tmp64;
			tmp64 = total_size / 1024;
			f = f * 100 / (float)(tmp64 + 1);
			pdwm->getProCtrl()->SetPos((int)f);

			diffsize = sdt.currsize - lastsize + 1;
			tmp64    = sdt.filesize - sdt.currsize;
			sdt.timeRemains = (int)tmp64 / diffsize;
			
			etimeHour   = sdt.timeElapsed / 3600;
			etimeMinute = (sdt.timeElapsed - etimeHour * 3600) / 60;
	 		etimeSeconds= sdt.timeElapsed - etimeHour * 3600 - etimeMinute * 60;

			rtimeHour   = sdt.timeRemains / 3600;
			rtimeMinute = (sdt.timeRemains - rtimeHour * 3600) / 60;
			rtimeSeconds= sdt.timeRemains - rtimeHour * 3600 - rtimeMinute * 60;

			f = size2float(sdt.currsize, c);
			sprintf(buf, "%.2f %c", f, c);
			plist->SetItemText((int)idx, idx_dwsize, buf);

			if (sdt.currsize == sdt.filesize)
			{
				strcpy(buf, "100%");
			}
			else
			{
				tmp64 = sdt.currsize / 1024;
				f = (float)tmp64;
				tmp64 = sdt.filesize / 1024;
				f = f * 100 / (float)(tmp64 + 1);
				sprintf(buf, "%.2f%%", f);
			}
			plist->SetItemText((int)idx, idx_ratio, buf);

			sprintf(buf, "%.2f k/s", (float)diffsize / g_1K / timesleep);
			plist->SetItemText((int)idx, idx_speed, buf);

			sprintf(buf, "%02d:%02d:%02d", etimeHour, etimeMinute, etimeSeconds);
			plist->SetItemText((int)idx, idx_elapsed, buf);

			sprintf(buf, "%02d:%02d:%02d", rtimeHour, rtimeMinute, rtimeSeconds);
			plist->SetItemText((int)idx, idx_remains, buf);

			lastsize = sdt.currsize;
		}

		total_dwsize += sdt.currsize;
		if (sdt.currsize == sdt.filesize)
		{
			plist->SetItemText((int)idx, idx_ratio, "100%");
		}
		tmp64 = sdt.filesize / 1024 / (sdt.timeElapsed + 1);
		sprintf(buf, "%d k/s", (int)tmp64);
		plist->SetItemText((int)idx, idx_speed, buf);

		sprintf(buf, "finished: %llu/%llu in %ds, average: %d k/s",
			sdt.currsize, sdt.filesize, sdt.timeElapsed, (int)tmp64);
		pdwm->log(buf);

		//close threads
		::WaitForMultipleObjects(threadCount, pthdl, TRUE, INFINITE);
		for (int i = 0; i < threadCount; i++)
		{
			::CloseHandle(pthdl[i]);
		}	

		//merge files
		pdwm->log("start to merge files");
		pdwm->combineFiles(vfi[idx].basename.c_str());

		++idx;
	}
	if (pdwm->downloading())
	{
		PostMessage(pdwm->getMsgHwnd(), WM_DOWNLOAD_FINISHED, 0, 0);
	}
	return 0;
}

int DwManager::startDownload(vector<file_info>& vfiles, const char* folder)
{
	m_dwfiles = vfiles;
	string tmpFolder = m_dstFolder;
	
	if (folder == NULL)
	{
		m_tmpFolder = tmpFolder;
		tmpFolder.append("\\tmp");
		CreateDirectory(tmpFolder.c_str(), NULL);
	}
	else
	{
		tmpFolder.append("\\");
		tmpFolder.append(folder);
		m_tmpFolder = tmpFolder;
		CreateDirectory(tmpFolder.c_str(), NULL);
		tmpFolder.append("\\tmp");
		CreateDirectory(tmpFolder.c_str(), NULL);
	}
	if (m_dwfiles.size() == 0)
	{
		return -2;
	}
	m_downloading = 1;
	m_dwThread = (HANDLE*)::_beginthreadex(NULL, 0, &pt_dw, (void*)this, 0, NULL);
	return 0;
}

void DwManager::dwFinished()
{
	m_downloading = 0;
	if (m_dwThread == INVALID_HANDLE_VALUE)
	{
		return;
	}
	::WaitForSingleObject(m_dwThread, INFINITE);
	::CloseHandle(m_dwThread);
	m_dwThread = INVALID_HANDLE_VALUE;
}
