#include <stdarg.h>
#include "common.h"

void set_timeout(int time_out)
{
	alarm(time_out);
}

int trim_string(char *p)
{
	if (p == NULL)
	{
		return 0;
	}

	char *pt = p;
	while (*pt == ' ' || *pt == '\t')
	{
		pt++;
	}
	int len = strlen(pt);
	char flag = 1;
	while (len && flag)
	{
		switch (pt[len - 1])
		{
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			len--;
			break;
		default:
			flag = 0;
			break;
		}
	}
	pt[len] = 0;
	if (p != pt)
	{
		int i = 0;
		for (i = 0; i < len; i++)
		{
			p[i] = pt[i];
		}
		p[i] = 0;	
	}
	return 0;
}

int get_str_pose(char *p, char flag, int * pose, int size)
{
	if (p == NULL)
	{
		return 0;
	}

	int n = 0;
	pose[n] = -1;
	n++;

	int i;
	for (i = 0; p[i]; i++)
	{
		if (p[i] == flag)
		{
			pose[n] = i;
			n++;
		}

		if (n >= size)
		{
			break;
		}
	} 

	pose[n] = i;
	return n;
}

int get_pose_str(char *p, int *pose, int number, char *buf, int size)
{
	buf[0] = 0;	

	if (number + 1 >= size)
	{
		return 0;
	}

	int n = pose[number + 1] - pose[number] - 1;
	strncpy(buf, p + pose[number] + 1, n);
	buf[n] = 0;
	return strlen(buf); 
}


int get_pose_str(char *p, int number, char *buf, char flag)
{
	int pose[256];
	int ret = get_str_pose(p, flag, pose, 256);
	if (number > ret)
	{
		return -1;
	}

	return get_pose_str(p, pose, number, buf, 256);	
}

////////////////////socket///////////////////////
int sock_read(int sock, char *buff, int bufLen)
{
	if (bufLen == 0)
	{
		return 0;
	}
	memset(buff, 0, bufLen);
	int len = 0;
	do
	{
		len = recv(sock, buff, bufLen, 0);
		if (len <= 0)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				write_log("%d, sock read :[%d][%s]", len, errno, strerror(errno));			
				continue;
			}
			write_log("%d, sock read error:[%d][%s]", len, errno, strerror(errno));			
		}
	}
	while (0);
	return len;
}


int sock_read(int sock, char *buff, int bufLen, int ntimeout)
{
	fd_set fds;
	int readret, selrtn;
	struct timeval timeout;

	timeout.tv_sec = ntimeout;
	timeout.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	selrtn = select(sock + 1, &fds, NULL, NULL, &timeout);
	if (selrtn < 0)
	{
		write_log("select error:[%d][%s]", errno, strerror(errno));
		return -1;
	}

	if (selrtn == 0)
	{
		write_log("select timeout:[%d][%s]", errno, strerror(errno));		
		return 0;
	}	

	return sock_read(sock, buff, bufLen);
}

int sock_send(int sock, char *buf, int nbyte)
{
	//write_log("we send:[%s]", buf);
	int n = 0;	
	char *sendbuf = (char *)buf;

	while (nbyte > 0)
	{
		set_timeout(60);
		n = write(sock, sendbuf, nbyte);
		set_timeout(0);

		if (n <= 0)
		{
			write_log("send: errno:%d, strerror:%s", errno, strerror(errno));			
			if (errno != EAGAIN && errno != EINTR)
			{
				//close(sock);
				return -1;
			}
			continue;
		}
		else
		{
			nbyte -= n;
			sendbuf = sendbuf + n;
		}
	}

	return 0;
}

int sock_open(const char *host, int clientPort)
{
	int sock = -1;	

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		write_log("create sock failed");	
		return -1;
	}
	//avoid 0, why we can stdin
	if (sock == 0)
	{
		int socktmp = sock;
		sock = socket(AF_INET, SOCK_STREAM, 0);
		close(socktmp);
	}

	unsigned long inaddr;
	struct sockaddr_in ad;

	memset(&ad, 0, sizeof(ad));
	ad.sin_family = AF_INET;
	ad.sin_port = htons(clientPort);

	if (inet_aton(host, &ad.sin_addr))
	{
		if (::connect(sock, (struct sockaddr *) &ad, sizeof(ad)) < 0)
		{
			close(sock);
			write_log("host [%s:%d] is unreachable", host, clientPort);
			return -1;
		}
	}
	else
	{
		struct hostent *hp = gethostbyname((char*)host);
		if (hp == NULL)
		{
			close(sock);			
			write_log("get host by name [%s] failed", host);
			return -1;
		}

		if(hp->h_length != 4 && hp->h_length != 8)
		{
			close(sock);			
			write_log("get host by name [%s] failed", host);
			return -1;
		}

		struct sockaddr_in **pptr = (struct sockaddr_in **)hp->h_addr_list;
		for(; *pptr != NULL; pptr++)
		{
			memcpy(&ad.sin_addr, *pptr, sizeof(struct in_addr));
			if (connect(sock, (struct sockaddr *) &ad, sizeof(ad)) == 0)
			{
				break; 
			}
		}

		if(*pptr == NULL)
		{
			close(sock);			
			write_log("get host by name [%s] failed", host);
			return -1;
		}
	}

	set_timeout(0);
	return sock;
}

int sock_cmd(int sock, char *fmt, ... )
{
	char buf[1024];
	va_list va;
	va_start( va, fmt );
	vsnprintf(buf, sizeof(buf) - 1 , fmt, va );
	va_end( va );

	strcat(buf, "\r\n");
	return sock_send(sock, buf, strlen(buf));
}

////////////////////////log section////////////////
#define LOG_FATAL 0
#define LOG_ERROR 1 
#define LOG_INFO  2
#define LOG_DEBUG 3

#define LOG_PATH "/tmp/fs"
#define CFG_FILE "/tmp/fs"


MyLog::MyLog()
{
	m_fp = NULL;
	m_level = LOG_INFO;
	strcpy(m_name, "default");
	sprintf(m_fullname, "%s/default.log", LOG_PATH);
}

MyLog::~MyLog()
{
	close();
}

MyLog* g_log = NULL;
MyLog* getLog()
{
	if (g_log == NULL)
	{
		g_log = new MyLog();
	}
	return g_log;
}

int MyLog::open(char *log_file)
{
	close();
	struct stat st;
	if (stat(LOG_PATH, &st) == -1)
	{
		char buf[256];
		sprintf(buf, "mkdir -p %s", LOG_PATH);
		system(buf);
	}

	time_t t = time(NULL);
	struct tm * tt = localtime(&t);

	//backup this name
	strcpy(m_name, log_file);
	sprintf(m_fullname, "%s/%s-%04d%02d%02d.log", LOG_PATH, m_name, tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday);	

	m_fp = fopen(m_fullname, "a");
	return m_fp ? 0 : -1;
}

void write_log(const char *fmt, ...)
{
	FILE* fp = getLog()->getFP();
	char* buf = getLog()->getBuf();

	fprintf(fp, "%s", buf);
	va_list	ap;
	va_start(ap, fmt);
	vfprintf(fp, fmt, ap);
	va_end (ap);
	fprintf(fp, "\n");
	fflush(fp);
}

/////////////////////////////////////////////////////////////////////////////////
int timeout_connect(int s, struct sockaddr* saddr, int size, int timeout) 
{ 
	int oldflag = fcntl(s, F_GETFL);
	fcntl(s, F_SETFL, O_NONBLOCK); 
	int ret = connect(s, saddr, size);
	if (ret == 0)
	{
		fcntl(s, F_SETFL, oldflag);
		return 0;
	}

	if (errno != EINPROGRESS)
	{
		return -1;
	}

	struct timeval tv; 
	fd_set writefds; 
	tv.tv_sec = timeout; 
	tv.tv_usec = 0; 
	FD_ZERO(&writefds); 
	FD_SET(s, &writefds); 

	if (select(s + 1, NULL, &writefds, NULL, &tv) <= 0)
	{
		return -1;
	}

	socklen_t len = sizeof(int);
	int error = 0; 
	getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len); 
	if(error == 0)
	{
		fcntl(s, F_SETFL, oldflag);
		return 0;
	}
	return -1;
}


int sock_open(const char *host, int clientPort, int timeout)
{
	int sock = -1;	

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		write_log("create sock failed");	
		return -1;
	}

	unsigned long inaddr;
	struct sockaddr_in ad;

	memset(&ad, 0, sizeof(ad));

	ad.sin_family = AF_INET;
	ad.sin_port = htons(clientPort);

	if (inet_aton(host, &ad.sin_addr))
	{
		if (timeout_connect(sock, (struct sockaddr *) &ad, sizeof(ad), timeout) < 0)
		{
			close(sock);
			write_log("host [%s:%d] is unreachable", host, clientPort);
			return -1;
		}
	}
	else
	{
		struct hostent *hp = gethostbyname((char*)host);

		if (hp == NULL)
		{
			close(sock);
			write_log("get host by name [%s] failed", host);
			return -1;
		}

		if(hp->h_length != 4 && hp->h_length != 8)
		{
			close(sock);			
			write_log("get host by name [%s] failed", host);
			return -1;
		}

		struct sockaddr_in **pptr = (struct sockaddr_in **)hp->h_addr_list;
		for(; *pptr != NULL; pptr++)
		{
			memcpy(&ad.sin_addr, *pptr, sizeof(struct in_addr));
			if (timeout_connect(sock, (struct sockaddr *) &ad, sizeof(ad), timeout) == 0)
			{
				break; 
			}
		}

		if(*pptr == NULL)
		{
			close(sock);			
			write_log("get host by name [%s] failed", host);
			return -1;
		}
	}

	return(sock);
}

void daemon_start(void)
{
	if (fork())
	{
		exit(0);
	}
	setsid();	

	if (fork())
	{
		exit(0);
	}
	//chdir("/");
	umask(0);
	close(0);
	close(1);
	close(2);
	open("/dev/null", O_RDONLY);
	dup2(0, 1);
	dup2(0, 2);
}

