#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <sys/stat.h>

typedef long long int64;
////////////string section
int trim_string(char *p);
int64 str_i64(const char*p);

int socket_init(int version_high, int version_low);
int socket_cleanup();

#define BUFSIZE          2048 //more than MTU
#define UDP_PACKAGE_SIZE 1280 //less than MTU

#define SOCKERR_TIMEOUT     0
#define SOCKERR_CONNERR    -1
#define SOCKERR_SENDERR    -2
#define SOCKERR_RECVERR    -3

//////////////////////////////////////////////////////////////////////////
//pub routines for windows
void socket_init();
void socket_end();
void socket_close(SOCKET&);


//////////////////////////////////////////////////////////////////////////
class SocketBase
{
public:
	SocketBase(int timeout = 30, int bufsize = BUFSIZE);
	virtual ~SocketBase();

protected:
	SOCKET   m_fd;
	int      m_timeout;
	int      m_errno;
	int      m_connected;
public:
	SOCKET sockfd()
	{
		return m_fd;
	}
	void setTimeout(int timeout)
	{
		m_timeout = timeout;
	}
	void setBlock(int block)
	{
		//
	}
	void setConnected(int con)
	{
		m_connected = con;
	}
	int isConnected()
	{
		return m_connected;
	}
	void close()
	{
		if (m_fd != INVALID_SOCKET)
		{
			socket_close(m_fd);
		}
		setConnected(0);
	}

protected:
	inline int waitdata();
};

int SocketBase::waitdata()
{
	fd_set  readfds;
	struct timeval  tv;
	tv.tv_usec = 0L;
	tv.tv_sec = m_timeout;

	FD_ZERO (&readfds);
	FD_SET (m_fd, &readfds);

	int ret = select(0, &readfds, NULL, NULL, &tv);
	m_errno = errno;
	if (ret == 0)
	{
		return SOCKERR_TIMEOUT;
	}
	if (ret < 0)
	{
		setConnected(0);
		return SOCKERR_CONNERR;
	}
	return FD_ISSET(m_fd, &readfds);
}

//for reading a line, we can use fgets in linux
class SockBuffer
{
public:
	SockBuffer(int size);
	~SockBuffer();	

private:
	char* m_buf;
	int   m_size;
	int   m_head;
	int   m_tail;
	int   m_len;

public:
	char* getPtr()
	{
		return m_buf + m_tail;
	}
	int getLen()
	{
		return m_len;
	}
	int getSize()
	{
		return m_size;
	}
	char* getBuf()
	{
		return m_buf;
	}
	inline int transfer(char* buf, int buflen);
	void reset(int len = 0)
	{
		m_tail = 0;
		m_head = len;
		m_len  = len;
	}
};

#define BUFSIZE          2048 //more than MTU
#define UDP_PACKAGE_SIZE 1280 //less than MTU

#define SOCKERR_TIMEOUT     0
#define SOCKERR_CONNERR    -1
#define SOCKERR_SENDERR    -2
#define SOCKERR_RECVERR    -3

//////////////////////////////////////////////////////////////////////////
//pub routines for windows
void socket_init();
void socket_end();
void socket_close(SOCKET&);

//////////////////////////////////////////////////////////////////////////
class TcpSocket : public SocketBase
{
public:
	TcpSocket(int timeout = 30, int bufsize = BUFSIZE);
	~TcpSocket();

public:
	//sending
	int send(const char* buf, int nByte);
	int send(const char* pmsg)
	{
		return send(pmsg, (int)strlen(pmsg));
	}
	int sendfile(const char* filename, int offset, int len);

	//receiving
	int recv(char* buf,       int bufLen);
	int recvn(char* buf, int nByte);
	int readline(char* buf, int buflen);

	//connection
	int start_service(DWORD port);
	SOCKET accept();
	int connect(const char* host, WORD port, int timeout = 30);

	//others
	void attach(SOCKET fd)
	{
		m_buffer->reset(0);
		socket_close(m_fd);
		m_fd = fd;
	}

private:
	SockBuffer* m_buffer;
};

//////////////////////////////////////////////////////////////////////////
int SockBuffer::transfer(char* buf, int buflen)
{
	int n = (m_len < buflen) ? m_len : buflen;
	memcpy(buf, m_buf + m_tail, n);

	m_tail = m_tail + n;
	m_len = m_head - m_tail;
	if (m_len == 0)
	{
		m_tail = 0;
		m_head = 0;
	}
	return n;
}

#endif


