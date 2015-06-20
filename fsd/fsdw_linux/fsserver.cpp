#include <sys/param.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termio.h>
#include <iostream>
#include <fstream>
#include <dirent.h>

#include <string>
#include <vector>
#include <algorithm>
using std::vector;
using std::string;
#include "common.h"

#define WELLCOME_MSG "File Server 0.3, Type "YELLOW"Help"WHITE" to get more information."
int str_endwith(const char* p, char c)
{
	size_t len = strlen(p);
	if (len == 0)
	{
		return 0;
	}
	return p[len - 1] == c;
}

enum e_file_type 
{
	FS_FILE = 0,
	FS_FOLDER,
	FS_UNKNOWN
};

struct FileObject
{
	FileObject(const char* name = NULL);
	FileObject(const char* folder, const char* name);

	int    m_type;
	int    m_status;
	int64  m_size;
	string m_fullname;
	string m_basename;
	time_t m_lastmodify;

	void reset()
	{
		m_fullname = "";
		m_basename = "";
		m_lastmodify = 0;
		m_size = -1;
		m_type = FS_UNKNOWN;
	}
	#define CHECK_FILE \
	if (m_type == FS_UNKNOWN)\
	{\
		getInfo();\
	}
	const char* fullname()
	{
		return m_fullname.c_str();
	}
	const char* basename()
	{
		return m_basename.c_str();
	}
	int isFolder()
	{
		CHECK_FILE
		return m_type == FS_FOLDER;
	}
	time_t lastmodify()
	{
		CHECK_FILE
		return m_lastmodify;
	}
	int64 size()
	{
		CHECK_FILE
		return m_size;
	}
	int exists()
	{
		CHECK_FILE
		return m_size != -1;
	}

	void setName(const char* name, const char* folder = NULL);

 private:
	int getInfo();
};
FileObject::FileObject(const char* name)
{
	setName(name);
}

FileObject::FileObject(const char* folder, const char* name)
{
	setName(name, folder);
}

void FileObject::setName(const char* name, const char* folder)
{
	reset();
	if (name == NULL)
	{
		return;
	}
	if (folder)
	{
		if (name[0] == '/')
		{
			m_basename = name + 1;
		}
		else
		{
			m_basename = name;
		}
		m_fullname = folder;
		if (!str_endwith(folder, '/'))
		{
			m_fullname.append("/");
		}
		m_fullname += m_basename;
		return;
	}
	m_fullname = name;
	const char* pt = strrchr(name, '/');
	if (pt == NULL)
	{
		m_basename = name;
	}
	else
	{
		m_basename = pt + 1;
	}
}

int FileObject::getInfo()
{
	struct stat64 statbuf;
	if (stat64(m_fullname.c_str(), &statbuf) < 0)
	{
		return -1;
	}
	m_lastmodify = statbuf.st_mtime;
	m_size = statbuf.st_size;
	m_type = S_ISDIR(statbuf.st_mode) ? FS_FOLDER : FS_FILE;
	return 0;
}

struct DWCfg
{
	char srcname[256];
	char tarname[256];
	char tmp[256];
	int  isfolder;
	long long filesize;
};

struct DWCfg g_dwcfg;

/////////change file name////////////
char* trim_filename(char* filename)
{
	trim_string(filename);

	//switch slash
	int i = 0;
	while (filename[i])
	{
		if (filename[i] == '\\')
		{
			filename[i] = '/';
		}
		i++;
	}
	return filename;
}

// get base name
char* get_lastname(char* filename)
{
	int i = 0;
	int last_char = 0;
	while (filename[i])
	{
		if (filename[i] == '/')
		{
			last_char = i;
		}
		i++;
	}
	return filename + last_char;
}

/////////////////////////////////////////

int file_cmp(const FileObject& f1, const FileObject& f2)
{
	//return f1.m_basename < f2.m_basename;
	if (f1.m_type == f2.m_type)
	{	
		int ret = strcasecmp(f1.m_basename.c_str(), f2.m_basename.c_str());
		return ret < 0;
	}
	return f1.m_type > f2.m_type;	
}

int _do_ls(const char* folder, vector<FileObject>& vf)
{
	vf.clear();
	DIR* dir = opendir(folder);
	if (dir == NULL)
	{
		return 0;
	}

	struct stat st;
	struct dirent * ptr;
	while((ptr = readdir(dir)) != NULL)
	{
		FileObject sf(folder, ptr->d_name);
		if (!sf.exists())
		{
			continue;
		}
		vf.push_back(sf);
	}

	closedir(dir);
	sort(vf.begin(), vf.end(), file_cmp);

	return vf.size();
}

int do_cmd_ls(int remotefd, char* args);

const int g_1G = 1024 * 1024 * 1024;
const int g_1M = 1024 * 1024;
const int g_1K = 1024;

int do_cmd_dir(int remotefd, char* args)
{
	char filename[256];
	strcpy(filename, args);
	trim_filename(filename);

	//find file
	struct stat64 statbuf;
	if (stat64(filename, &statbuf) < 0)
	{
		sock_cmd(remotefd, "-ERR Cannot find (%s)", filename);	
		return 0;
	}
	if (!S_ISDIR(statbuf.st_mode)) 
	{
		return do_cmd_ls(remotefd, args);
	}
	
	vector<FileObject> vf;
	_do_ls(filename, vf);

	string strcontent("");
	char buf[128];
	int total = 0;
	for (size_t i = 0; i < vf.size(); i++)
	{
		if (strcmp(vf[i].basename(), ".") == 0)
		{
			continue;
		}
		total++;
		strcontent = strcontent + vf[i].basename();
		if (vf[i].isFolder())
		{
			strcontent.append(":1");
		}
		else
		{
			strcontent.append(":0");
		}
		sprintf(buf, ":%llu", vf[i].size());
		strcontent.append(buf);
		sprintf(buf, ":%u\n", vf[i].lastmodify());
		strcontent.append(buf);
	}
	sock_cmd (remotefd, "+OK %d", total);//strcontent.length());
	sock_send(remotefd, (char*)strcontent.c_str(), strcontent.length());
	return 0;
}

int do_cmd_ls(int remotefd, char* args)
{
	char filename[256];
	strcpy(filename, args);
	trim_filename(filename);

	//find file
	struct stat64 statbuf;
	if (stat64(filename, &statbuf) < 0)
	{
		sock_cmd(remotefd, "-ERR Cannot find (%s)", filename);	
	}
	else
	{
		time_t t = statbuf.st_ctime;
		long long s = statbuf.st_size;

		char buft[64];
		struct tm * tt = localtime(&t);

		sprintf(buft, YELLOW"%04d-%02d-%02d %02d:%02d:%02d",
			tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday,
			tt->tm_hour + 1, tt->tm_min + 1, tt->tm_sec + 1);	
	
		sock_cmd(remotefd, "SIZE: %llu(%lluK), %s", s, s / 1024, buft); 
	}
	return 0;
}

int do_cmd_size(int remotefd, char* args)
{
	char filename[256];
	strcpy(filename, args);
	trim_filename(filename);

	g_dwcfg.isfolder = 0;
	//find file
	struct stat64 statbuf;
	if (stat64(filename, &statbuf) < 0)
	{
		sock_cmd(remotefd, "-ERR Cannot find (%s)", filename);
		return 0;
	}
	if (S_ISDIR(statbuf.st_mode)) 
	{
		sock_cmd(remotefd, "-ERR this is a folder");
		return 0;
		char cmd[1024];	
		g_dwcfg.isfolder = 1;
		sprintf(g_dwcfg.tarname, "%s/%s.tar", g_dwcfg.tmp, get_lastname(filename));
		sprintf(cmd, "tar cvf %s %s", g_dwcfg.tarname, filename);
		write_log(cmd);
		system(cmd);	
		if (stat64(g_dwcfg.tarname, &statbuf) < 0)
		{
			sock_cmd(remotefd, "-ERR Cannot find (%s)", filename);	
			return 0;
		}
	}
	sock_cmd(remotefd, "+OK %llu", statbuf.st_size);
	return 0;
}

int do_cmd_quit(int remotefd, char* args)
{
	if (g_dwcfg.isfolder)
	{
		char cmd[1024];
		sprintf(cmd, "/bin/rm -f %s", g_dwcfg.tarname);
		write_log(cmd);
		system(cmd);
	}
	sock_cmd(remotefd, "bye");
	return 0;
}

char* parse_cmd_get(char* tmp)
{
	trim_string(tmp);
	char* pt = strrchr(tmp, ' ');
	if (pt)
	{
		*pt = 0;
		return pt + 1;
	}
	return tmp;
}

int do_cmd_get(int remotefd, char* args)
{
	char tmp[256];
	char filename[256];
	long long offset = 0;
	long long length = 0;
	
	strcpy(tmp, args);

	length = atoll(parse_cmd_get(tmp));
	offset = atoll(parse_cmd_get(tmp));
	strcpy(filename, tmp);//parse_cmd_get(tmp));
	trim_filename(filename);

	write_log("get (%s) %llu %llu", filename, offset, length);
	//find file
	struct stat64 statbuf;
	if (stat64(filename, &statbuf) < 0)
	{
		sock_cmd(remotefd, RED"Error: "YELLOW"Cannot find (%s)"WHITE, filename);	
		return -1;
	}
	else
	{
		FILE* fp = fopen64(filename, "rb");
		if (fp == NULL)
		{
			sock_cmd(remotefd, RED"Error: "YELLOW"Cannot open (%s)"WHITE, filename);	
			return -1;
		}
		write_log("start to transfer...");
		int hfp = fileno(fp);

		lseek64(hfp, offset, SEEK_SET);
		long long total = length;
		char buf[1024];
		while (total)
		{
			int n = fread(buf, 1, sizeof(buf), fp);
			if (n <= 0)
			{
				break;
			}
			sock_send(remotefd, buf, n);
			total -= n;
		}
		write_log("ok, send %llu/%llu", offset, length);
		fclose(fp);
	}
	
	return 0;
}

int new_child_process(int socketfd, int remotefd)
{
	int child_pid = 0;	
	int ret = 0;
	
	child_pid = fork();
	
	if(child_pid < 0)
	{
	    write_log("error fork");
	    return -1;
	}
	
	if(child_pid == 0) /* Child */
	{
		close(socketfd); 
		char buff[8192];	
		int ret = -1;
		int stage = 0;
		struct stat64 statbuf;
	
		FILE* fpSock = fdopen(remotefd, "r");
		setbuf(fpSock, (char *)0);

		sock_cmd(remotefd, WELLCOME_MSG);
		while (1)
		{
			if (fgets(buff, sizeof(buff), fpSock) == NULL)
			{
				break;
			}
	
			write_log("cmd:%s", buff);	
			if (strncasecmp(buff, "help", 4) == 0)
			{
				stage = 1;
				sock_cmd(remotefd, "Created by Myao, Help to download file from map server.");
				sock_cmd(remotefd, "ChangeList: 2015-05-11, support files bigger than 3GB.");
				sock_cmd(remotefd, "Commands:");
				sock_cmd(remotefd, "ls	<file name>");
				sock_cmd(remotefd, "get	<file name> [offset][length]");
				sock_cmd(remotefd, "put	<file name> (not ready)");
				sock_cmd(remotefd, "quit");

				continue;
			}			
			if (strncasecmp(buff, "ls", 2) == 0)
			{

				//FILE* fp = mig_init_sock_fp(remotefd);
				do_cmd_ls(remotefd, buff + 2);
				continue;
			}	
			if (strncasecmp(buff, "size", 4) == 0)
			{

				//FILE* fp = mig_init_sock_fp(remotefd);
				do_cmd_size(remotefd, buff + 4);
				continue;
			}	
			if (strncasecmp(buff, "get", 3) == 0)
			{

				do_cmd_get(remotefd, buff + 3);
				continue;
			}	
			if (strncasecmp(buff, "dir", 3) == 0)
			{

				do_cmd_dir(remotefd, buff + 3);
				continue;
			}	
			if (strncasecmp(buff, "quit", 4) == 0)
			{
				stage = 1;
				do_cmd_quit(remotefd, buff + 4);
				close(remotefd);
				break;	
			}	
			sock_cmd(remotefd, "unknown command");
		}
		
		close(remotefd);
	  exit(0);
	}
	
	close (remotefd);
	return 0;
}


int init_server()
{
	int client;
	int sockfd, newsockfd, remotefd;
	struct sockaddr_in cli_addr, serv_addr, remote_addr;
	int listen_count = 5;

	int port = cfg_getn("port", 18110);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int on = 0;
	//setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	setsockopt(sockfd, IPPROTO_TCP,	SO_REUSEADDR, (char *)&on, sizeof(int));
	
	memset((void *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
    
	int retrytime = 1;

	while (bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		write_log("Error binding...Retry...");
		retrytime *= 2;
		sleep(retrytime);
	}
	if (listen(sockfd, listen_count) < 0)
	{
		write_log("Error listening socket.");
		exit(1);
	}

	signal(SIGCHLD, SIG_IGN);	
	
	while(1)
	{
		socklen_t clien = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clien);
		
		if (newsockfd < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			
			write_log("Error accept.");
			exit(1);
		}
		
		new_child_process(sockfd, newsockfd);
	}
	
	//close_mem();
	return(0);

}

int main(int argc, char **argv)
{
	cfg_open("fs.conf");
	strcpy(g_dwcfg.tmp, cfg_get("tmp", "/home/myao"));

	daemon_start(); 
	
	getLog()->open("fs");

	init_server();

	return 0;
}

