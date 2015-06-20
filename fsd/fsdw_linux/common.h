#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/types.h>	//basic system data types
#include <sys/socket.h>	//basic socket definitions
#include <sys/time.h>	//timeval for select
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <time.h>
#include <netinet/in.h>	//sockaddr_in and other internet defines
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

//color
#define RED	"\033[40;31m"
#define GREEN	"\033[40;32m"
#define YELLOW	"\033[40;33m"
#define BLUE	"\033[40;34m"
#define PURPLE	"\033[40;35m"
#define CYRN	"\033[40;36m"
#define WHITE	"\033[40;37m"

typedef long long int64;
////////////string section
int trim_string(char *p);

int get_str_pose(char *p, char flag, int * pose, int size);
int get_pose_str(char *p, int number, char *buf, char flag);
int get_pose_str(char *p, int *pose, int number, char *buf, int size);

////////////////network section
void set_timeout(int);
int sock_connect(const char *ip, int port);
int sock_open(const char *host, int clientPort, int timeout);
int sock_open(const char *host, int clientPort);
int sock_read(int sock, char *buf, int len);
int sock_read(int sock, char *buf, int len, int timeout);
int sock_send(int sock, char *buf, int len);
int sock_cmd(int sock, char *fmt, ... );


void write_log(const char *fmt, ...);
class MyLog
{
 public:
	MyLog();
	~MyLog();

 private:
	int m_level;
	int m_lastDay;
	FILE* m_fp;
	char m_buf[1024];
	char m_name[64];
	char m_fullname[256];

 public:
	void setLevel(int level)
	{
		m_level = level;
	}
	void close()
	{
		if (m_fp)
		{
			fclose(m_fp);
		}
		m_fp = NULL;
	}
	int open(char* name);
	int open()
	{
		return open(m_name);
	}
	FILE* getFP()
	{
		return m_fp;
	}
	char* getBuf()
	{
		return m_buf;
	}
};

MyLog* getLog();
/////////////////////////

int cfg_open(char* file, int size = 256);
char* cfg_get(char* key, const char* pDefault);
int cfg_getn(char* key, int nDefault);
////////////////////others
void daemon_start(void);

#endif


