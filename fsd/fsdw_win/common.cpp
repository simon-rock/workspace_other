#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "common.h"

enum SelectType
{
	ST_READ,
	ST_WRITE,
	ST_RW,
};

int sock_is_ready(int sock, int seconds, SelectType seltype = ST_READ)
{
	fd_set fds;
	int selrtn = 0;
	struct timeval timeout;

	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	switch(seltype)
	{
	case ST_READ:
		selrtn = select(0, &fds, NULL, NULL, &timeout);
		break;

	case ST_WRITE:
		selrtn = select(0, NULL, &fds, NULL, &timeout);
		break;

	case ST_RW:
		selrtn = select(0, &fds, &fds, NULL, &timeout);
		break;

	default:
		break;
	}

	return selrtn;
}

int64 str_i64(const char*p)
{
	char * endptr = NULL;
	int64 total = _strtoi64(p, &endptr, 10);
	return total;
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
	int len = (int)strlen(pt);
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

////////////////////socket///////////////////////
int socket_init (int version_high, int version_low)
{
	WORD version_requested = MAKEWORD (version_high, version_low);
	WSADATA wsa_data;
	return WSAStartup (version_requested, &wsa_data);
}

int socket_cleanup()
{
	return WSACleanup();
}

//////////////////////////////////////////////////////////////////////////
void socket_init()
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		fprintf(stderr,"WSAStartup failed!\n");
	}
}

void socket_end()
{
	WSACleanup();
}

void socket_close(SOCKET& sockfd)
{
	closesocket(sockfd);
	sockfd = INVALID_SOCKET;
}


SocketBase::SocketBase(int timeout, int bufsize)
{
	m_fd = INVALID_SOCKET;
	m_timeout = timeout;
}

SocketBase::~SocketBase()
{
	close();
}

//////////////////////////////////////////////////////////////////////////
SockBuffer::SockBuffer(int size)
{
	m_size = size;
	m_head = 0;
	m_tail = 0;
	m_len  = 0;
	m_buf  = new char[size];
}

SockBuffer::~SockBuffer()
{
	delete[] m_buf;
}

//////////////////////////////////////////////////////////////////////////
TcpSocket::TcpSocket(int timeout, int bufsize):SocketBase(timeout, bufsize)
{
	m_buffer = new SockBuffer(bufsize);
}

TcpSocket::~TcpSocket()
{
	delete m_buffer;
}

int TcpSocket::send(const char* buf, int nByte)
{
	int n = 0;      
	char *sendbuf = (char *)buf;

	while (nByte > 0)
	{       
		n = ::send(m_fd, buf, nByte, 0);
		m_errno = errno;
		if (n <= 0)
		{       
			if ((m_errno != EAGAIN) && (m_errno != EINTR))
			{       
				return SOCKERR_SENDERR;
			}       
			continue;
		}       
		else    
		{       
			nByte -= n;
			sendbuf = sendbuf + n;
		}       
	}       

	return nByte;
}

int TcpSocket::recv(char* buf, int bufLen)
{
	if (m_buffer->getLen())
	{
		return m_buffer->transfer(buf, bufLen);
	}

	int n = waitdata();
	if (n <= 0)
	{
		return n;
	}
	n = 0;
	do
	{
		n = ::recv(m_fd, buf, bufLen, 0);
		m_errno = errno;
		if (n < 0)
		{
			//log("ERR: sock read err:[%s]", strerror(m_errno));
			if (m_errno == EAGAIN || m_errno == EINTR)
			{
				continue;
			}
			setConnected(0);
			return SOCKERR_RECVERR;
		}
		else if (n == 0)
		{
			//log("INF: sock closed:[%s]", strerror(m_errno));			
			return SOCKERR_TIMEOUT;
		}
	}while (0);

	return n;
}

int TcpSocket::recvn(char* buf, int nByte)
{
	int offset = 0;
	if (m_buffer->getLen())
	{
		offset = m_buffer->transfer(buf, nByte);
		if (offset == nByte)
		{
			return offset;
		}
	}
	int n = 0;
	while (offset < nByte)
	{
		n = recv(buf + offset, nByte - offset);
		if (n <= 0)
		{
			return n;
		}
		offset += n;
	}

	return offset;
}

int TcpSocket::readline(char* buf, int bufsize)
{
	bufsize -= 1;
	int offset = 0;
	int nleft = bufsize;
	int i = 0, n = 0;
	while (offset < bufsize)
	{
		int len = m_buffer->getLen();
		if (len)
		{
			char* ptr = m_buffer->getPtr();
			i = 0;
			while (i < len && ptr[i] != '\n') i++;
			//0123n, i = 4, we need to move 5 bytes
			if (i < nleft)
			{
				if (i < len)
				{
					m_buffer->transfer(buf + offset, i + 1);
					offset += i;
					break;
				}
				else
				{
					m_buffer->transfer(buf + offset, i);
					offset += i;
					nleft -= i;
				}
			}
			else
			{
				m_buffer->transfer(buf + offset, nleft);
				offset += nleft;
				break;
			}
		}
		n = recv(m_buffer->getBuf(), m_buffer->getSize());
		if (n <= 0)
		{
			return -1;
		}
		m_buffer->reset(n);
	}

	buf[offset] = 0;
	//log("DBG: line:[%s]", buf);
	return offset;
}


int TcpSocket::connect(const char* host, WORD port, int timeout)
{
	m_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_fd < 0)
	{
		return -1;
	}
	struct sockaddr_in ad;

	memset(&ad, 0, sizeof(ad));
	ad.sin_family = AF_INET;
	ad.sin_port = htons(port);

	if (INADDR_NONE != (ad.sin_addr.S_un.S_addr = inet_addr(host)))
	{
		if (::connect(m_fd, (struct sockaddr *) &ad, sizeof(ad)) < 0)
		{
			socket_close(m_fd);
			return -1;
		}
	}
	else
	{
		struct hostent *hp = gethostbyname((char*)host);
		if (hp == NULL)
		{
			socket_close(m_fd);
			return -1;
		}

		if(hp->h_length != 4 && hp->h_length != 8)
		{
			socket_close(m_fd);
			return -1;
		}

		struct sockaddr_in **pptr = (struct sockaddr_in **)hp->h_addr_list;
		for(; *pptr != NULL; pptr++)
		{
			memcpy(&ad.sin_addr, *pptr, sizeof(struct in_addr));
			if (::connect(m_fd, (struct sockaddr *) &ad, sizeof(ad)) == 0)
			{
				break;
			}
		}

		if(*pptr == NULL)
		{
			socket_close(m_fd);
			return -1;
		}
	}
	setConnected(1);
	return 0;
}

SOCKET TcpSocket::accept()
{
	SOCKET fdsock = INVALID_SOCKET;
	fdsock = ::accept(m_fd, NULL, NULL);
	if (fdsock == INVALID_SOCKET)
	{
		//log("FATAL: accept error.");
	}

	return fdsock;
}

