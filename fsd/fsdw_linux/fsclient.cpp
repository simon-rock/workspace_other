#include "common.h"

//20M


#define THREADCOUNT 50
class DwManager
{
 public:
	DwManager(char* host, short port, int threadCount);
	~DwManager();

 private:
	int64 m_ptDwSize[THREADCOUNT];
	int m_threadCount;
	char m_host[64];
	short m_port;
	int64 m_filesize;
	char m_filename[256];
	char m_basename[256];
	int m_fd;

 public:
	char* getHost()
	{
		return m_host;
	}
	short getPort()
	{
		return m_port;
	}
	int getThreadCount()
	{
		return m_threadCount;
	}
	int64 getFileSize()
	{
		return m_filesize;
	}
	char* getFileName()
	{
		return m_filename;
	}
	char* getBaseName()
	{
		return m_basename;
	}
	int64 getDwSize(int threadNumber)
	{
		return m_ptDwSize[threadNumber];
	}
	void setDwSize(int threadNumber, int64 size)
	{
		m_ptDwSize[threadNumber] = size;
	}
	void setName(char* filename);
	int64 getSizeFromServer();
	void quit()
	{
		if (m_fd != -1)
		{
			close(m_fd);
			m_fd = -1;
		}
	}
};

DwManager::DwManager(char* host, short port, int threadCount)
{
	memset(m_ptDwSize, 0, sizeof(m_ptDwSize));
	m_threadCount = threadCount;
	strcpy(m_host, host);
	m_port = port;
	m_fd = -1;
}

void DwManager::setName(char* filename)
{
	strcpy(m_filename, filename);
	//special for map1
	if (strncmp(filename, "\\\\map1", 6) == 0)
	{
		strcpy(m_filename, filename + 6);
	}
	//end
	char*pt = strrchr(m_filename, '/');
	if (pt == NULL)
	{
		pt = strrchr(m_filename, '\\');
	}
	if (pt)
	{
		strcpy(m_basename, pt + 1);
	}
	else
	{
		strcpy(m_basename, m_filename);
	}
}

int64 DwManager::getSizeFromServer()
{
	if (m_fd != -1)
	{
		close(m_fd);
		m_fd = -1;
	}
	int sock = sock_open(m_host, m_port);
	if (sock <= 0)
	{
		printf("connect server failed\n");
		exit (1);
	}

	FILE* fpSock = fdopen(sock, "r");
	if (fpSock == NULL)
	{
		close(sock);
		return -2;
	}
	setbuf(fpSock, (char *)0);

	char buf[1024];
	if (fgets(buf, sizeof(buf), fpSock) == NULL)
	{
		close(sock);
		return -3;
	}
	printf("<=%s", buf);
		
	printf(GREEN"get %s\n"WHITE, m_filename);
	sock_cmd(sock, "size %s", m_filename);
	if (fgets(buf, sizeof(buf), fpSock) == NULL)
	{
		close(sock);
		return -4;
	}
	//if (strncmp(buf, "OK", 2) != 0)
	if (strncmp(buf, "+OK", 3) != 0) // simon
	{
		printf(RED"Error: %s", buf);
		close(sock);
		return -5;
	}
	// printf(YELLOW"file size: "WHITE"%s", buf + 3);
	printf(YELLOW"file size: "WHITE"%s", buf + 4); // simon
	// int64 total = atoll(buf + 3);
	int64 total = atoll(buf + 4); // simon
	if (total <= 0)
	{
		printf(RED"Error: "YELLOW"Wrong size:"WHITE"%d\n", total);
		close(sock);
		return -6;
	}

	//printf("file size: %llu\n", total);
	//close(sock);
	m_fd = sock;
	m_filesize = total;
	return m_filesize;	
}

DwManager* g_pDM = NULL;

void* pt_report(void* args)
{
	//return NULL;
	//pthread_detach(pthread_self());

	int64 current_size = 0;
	int64 last_size = 0;
	int timecost = 0;
	//time_t tb = time(NULL);
	int timeint = 2;
	int64 filesize = g_pDM->getFileSize();
	int filesize32 = filesize / 1024;
	while (current_size < filesize)
	{
		sleep(timeint);
		timecost += timeint;
		current_size = 0;
		for (int i = 0; i < THREADCOUNT; i++)
		{
			current_size += g_pDM->getDwSize(i);
		}
		int64 dsize = (current_size - last_size) / 1024 / timeint;
		int dsize32 = (int)dsize;
		int cursize32 = (int)current_size / 1024;
		if (dsize <= 0)
		{
			dsize = 1;
		}
		int timeExpected = (filesize32 - cursize32) / dsize;
		char* color = CYRN;
		if (dsize < 100)
		{
			color = RED;
		}
		else if (dsize < 200)
		{
			color = YELLOW;
		}
		else if (dsize < 300)
		{
			color = GREEN;
		}
		printf("speed:%s%u(k/s)"WHITE", finished/total:%u/%u(k) (%.2f%%), elapsed:%d(s), remains:%d(s)\n",
			color, dsize32, cursize32, filesize32, (float)cursize32 * 100 / filesize32, timecost, timeExpected);
		last_size = current_size;
	}

	printf("["GREEN"OK"WHITE"], size: %llu, cost: %d seconds, average: %d k/s\n",
		filesize, timecost, filesize / 1024 / timecost + 1);
	return NULL;
}

struct pt_dwdata
{
	int number;
	int64 offset;
	int64 length;
};

void* pt_download(void* args)
{
	struct pt_dwdata* pdp = (struct pt_dwdata*)args;
	//printf("thread: %d, offset: %d, length:%d\n", pdp->number, pdp->offset, pdp->length);

	int64 total = 0;
	int trying = 0;
	char buf[1024];

	char localname[256];
	sprintf(localname, ".tmp/%s_%d", g_pDM->getBaseName(), pdp->number);

	int64 dataLeft = pdp->length - total;
	FILE* fp = fopen(localname, "wb");

	char tmp[1024];
	int sock = -1;
	while (total < pdp->length)
	{
		sock = sock_open(g_pDM->getHost(), g_pDM->getPort());
		if (sock <= 0)
		{
			printf(RED"Error: "WHITE"connect server failed, retry: %d times\n", trying++);
			sleep(5);
			continue;
		}
		//welcome message
		
		FILE* fpSock = fdopen(sock, "r");
		setbuf(fpSock, (char *)0);
		fgets(buf, sizeof(buf), fpSock);
		printf("thread: "YELLOW"%d"WHITE", connect server "GREEN"OK\n"WHITE, pdp->number);

		dataLeft = pdp->length - total;
		sprintf(tmp, "get %s %llu %llu", g_pDM->getFileName(), pdp->offset + total, dataLeft);
		printf("%s\n", tmp);
		sock_cmd(sock, tmp);
		//printf("get %s %d %d\n", g_pDM->getFileName(), pdp->offset + total, dataLeft);

		int64 len;
		while (dataLeft)// < pdp->length)
		{	
			len = sizeof(buf) > dataLeft ? dataLeft : sizeof(buf);
			//printf("%d\n", len);
			int n = sock_read(sock, buf, (int)len);
			if (n <= 0)
			{
				printf("sock read return %d bytes\n", n);
				break;
			}
			fwrite(buf, n, 1, fp);
			dataLeft -= n;
			total += n;
			g_pDM->setDwSize(pdp->number, total);
		}
		close(sock);
		//printf("%d/%d\n", total, g_data.final_size);
	}
	fclose(fp);

	printf("thread: "YELLOW"%d"WHITE", my task is "GREEN"OK\n"WHITE, pdp->number);
	return NULL;
}

int combineFiles()
{
	char localname[256];
	char name[256];
	strcpy(name, g_pDM->getBaseName());

	int ptn = g_pDM->getThreadCount();
	char buf[10240];

	FILE* fp = fopen64(name, "wb");
	struct stat64 fst;

	printf("Combine tmp files to "YELLOW"(%s)\n"WHITE, name);
	for (int i = 0; i < ptn; i++)
	{
		sprintf(localname, ".tmp/%s_%d", name, i);
		FILE* fpt = fopen(localname, "rb");
		if (fpt == NULL)
		{
			fprintf(stderr, "cannot open %s\n", localname);
			exit (1);
		}
		stat64(localname, &fst);
		int64 dataLeft = fst.st_size;
		while (dataLeft)// < pdp->length)
		{	
			int n = sizeof(buf) > dataLeft ? dataLeft : sizeof(buf);
			//printf("%d\n", len);
			fread(buf, n, 1, fpt);
			fwrite(buf, n, 1, fp);
			dataLeft -= n;
		}
		fclose(fpt);
	}

	fclose(fp);	
	return 0;
}

int main(int argc, char* argv[])
{

	int nThread = 10;
	if (argc < 3)
	{
		printf("%s <host> <filename> [thread count, default=10]\n", argv[0]);
		return 0;
	}
	if (argc >= 4)
	{
		nThread = atoi(argv[3]);
	}

	getLog()->open("fc");
	system("mkdir .tmp >/dev/null 2>&1");
	
	//start
	g_pDM = new DwManager(argv[1], 18110, nThread);
	g_pDM->setName(argv[2]);

	if (g_pDM->getSizeFromServer() < 0)
	{
		return -1;
	}

	printf("start to downlaoding...\n");
	//start report
	pthread_t tid;
	pthread_create(&tid, NULL, pt_report, NULL);

	//how many threads?
	//what is the size for every thread?
	int threadCount = g_pDM->getThreadCount();
	int64 filesize = g_pDM->getFileSize();
	int64 pt_len = (filesize + threadCount) / threadCount;
	printf("----file size: %llu, every thread download: %llu\n", filesize, pt_len);

	struct pt_dwdata ptdw[THREADCOUNT];
	for (int i = 0; i < threadCount; i++)
	{
		ptdw[i].number = i;
		ptdw[i].offset = pt_len * i;
		ptdw[i].length = pt_len;
	}
	ptdw[threadCount - 1].length = filesize % pt_len;

	for (int i = 0; i < threadCount; i++)
	{
		//printf("%d, %d, %d\n", ptdw[i].number, ptdw[i].offset, ptdw[i].length);
	}

	//start
	pthread_t ptid[THREADCOUNT];
	for (int i = 0; i < threadCount; i++)
	{
		pthread_create(&ptid[i], NULL, pt_download, (void*)&ptdw[i]);
	}

	//finished
	for (int i = 0; i < threadCount; i++)
	{
		pthread_join(ptid[i], NULL);
	}

	pthread_join(tid, NULL);
	combineFiles();
	system("/bin/rm -rf .tmp");

	g_pDM->quit();
	return 0;
}

