#pragma once
#include <string.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
using std::string;
using std::wstring;
using std::stringstream;
using std::ifstream;
using std::fstream;
using std::ios;
using std::cout;
using std::endl;
#include "digest/md5_dgst.h"
#include "digest/sha.h"
namespace digest
{
#define SHA1_CTX  SHA_CTX
	class md5
	{
	public:
		md5():
		enable(false)
		{
			init();
		}
		~md5(){}

		void update(const void *data, size_t len)
		{
			if(!enable)
			{
				init();
			}
			MD5_Update(&s, data, len);
		}
		string get_digest()
		{
			char out[3] = {0};
			unsigned char hash[16] = {0};
			char digt[32 + 1] = {0};
			MD5_Final(hash, &s);
			for(int i=0; i<16; ++i)
			{
				sprintf ( out,"%02x", ( int )hash[i] );
				memcpy ( digt + 2 * i, out, 2 );
			}
			enable = false;
			return digt;
		}
	private:
		void init()
		{
			enable = true;

			MD5_Init(&s);
		}
		bool enable;
		MD5_CTX s;
	};

	class sha1
	{
	public:
		sha1():
		  enable(false)
		  {
			  init();
		  }
		  ~sha1(){}

		  void update(const void *data, size_t len)
		  {
			  if(!enable)
			  {
				  init();
			  }
			  SHA1_Update(&s, data, len);
		  }
		  string get_digest()
		  {
			  char out[3] = {0};
			  unsigned char hash[20] = {0};
			  char digt[40 + 1] = {0};
			  SHA1_Final(hash, &s);
			  for(int i=0; i<20; ++i)
			  {
				  sprintf ( out,"%02x", ( int )hash[i] );
				  memcpy ( digt + 2 * i, out, 2 );
			  }
			  enable = false;
			  return digt;
		  }
	private:
		void init()
		{
			enable = true;

			SHA1_Init(&s);
		}
		bool enable;
		SHA_CTX s;
	};

	class size
	{
	public:
		size():
		  enable(false)
		  {
			  init();
		  }
		  ~size(){}

		  void update(const void *data, long long len)
		  {
			  if(!enable)
			  {
				  init();
			  }
			  fsize += len;
		  }
		  string get_digest()
		  {
			  char digt[256] = {0};
#ifdef WIN32              
              sprintf_s ( digt, 256, "%x", fsize );
#else                          
              snprintf ( digt, 256, "%x", fsize );
#endif              
			  enable = false;
			  return digt;
		  }
	private:
		void init()
		{
			enable = true;
			fsize = 0;
		}
		bool enable;
		long long fsize;
	};

	class sha512
	{
	public:
		sha512():
		  enable(false)
		  {
			  init();
		  }
		  ~sha512(){}

		  void update(const void *data, size_t len)
		  {
			  if(!enable)
			  {
				  init();
			  }
			  SHA512_Update(&s, data, len);
		  }
		  string get_digest()
		  {
			  char out[3] = {0};
			  unsigned char hash[64] = {0};
			  char digt[128 + 1] = {0};
			  SHA512_Final(hash, &s);
			  for(int i=0; i<64; ++i)
			  {
				  sprintf ( out,"%02x", ( int )hash[i] );
				  memcpy ( digt + 2 * i, out, 2 );
			  }
			  enable = false;
			  return digt;
		  }
	private:
		void init()
		{
			enable = true;
			SHA512_Init(&s);
		}
		bool enable;
		SHA512_CTX s;
	};

	class sha256
	{
	public:
		sha256():
		  enable(false)
		  {
			  init();
		  }
		  ~sha256(){}

		  void update(const void *data, size_t len)
		  {
			  if(!enable)
			  {
				  init();
			  }
			  SHA256_Update(&s, data, len);
		  }
		  string get_digest()
		  {
			  char out[3] = {0};
			  unsigned char hash[32] = {0};
			  char digt[64 + 1] = {0};
			  SHA256_Final(hash, &s);
			  for(int i=0; i<32; ++i)
			  {
				  sprintf ( out,"%02x", ( int )hash[i] );
				  memcpy ( digt + 2 * i, out, 2 );
			  }
			  enable = false;
			  return digt;
		  }
	private:
		void init()
		{
			enable = true;
			SHA256_Init(&s);
		}
		bool enable;
		SHA256_CTX s;
	};
//////////////////////////////////////////////////////////////////////////
	enum digest_bit
	{
		FILESIZE = 0,
		MD5 = 16,
		SHA1 = 20,
		SHA256 = 32,
		SHA512 = 64
	};

#define DIGESTINFO(TYPE) \
	typedef struct _##TYPE##_info \
	{   typedef TYPE##_CTX _digest_date; \
		typedef int (* _digest_init)(_digest_date *c) ; \
		typedef int (* _digest_update)(_digest_date *c, const void *data, size_t len) ; \
		typedef int (* _digest_final)(unsigned char *md, _digest_date *c) ; \
		static _digest_init _init; \
		static _digest_update _update; \
		static _digest_final _final; \
	}TYPE##_info; \
	TYPE##_info::_digest_init TYPE##_info::_init = TYPE##_Init; \
	TYPE##_info::_digest_update TYPE##_info::_update = TYPE##_Update; \
	TYPE##_info::_digest_final TYPE##_info::_final = TYPE##_Final;
DIGESTINFO(MD5)
DIGESTINFO(SHA1)
DIGESTINFO(SHA256)
DIGESTINFO(SHA512)

typedef struct _FILESIZE_info{}FILESIZE_info;

#define DIGESTNAME(TYPE) TYPE##_info

template<typename T, unsigned int SIZE>
class digest_handle
	{
	public:
		digest_handle():
		  enable(false)
		  {
			  init();
		  }
		  ~digest_handle(){}

		  void update(const void *data, size_t len)
		  {
			  if(!enable)
			  {
				  init();
			  }
			  T::_update(&s, data, len);
		  }
		  string get_digest()
		  {
			  char out[3] = {0};
			  unsigned char hash[SIZE] = {0};
			  char digt[SIZE*2 + 1] = {0};
			  T::_final(hash, &s);
			  for(int i=0; i<SIZE; ++i)
			  {
				  sprintf ( out,"%02x", ( int )hash[i] );
				  memcpy ( digt + 2 * i, out, 2 );
			  }
			  enable = false;
			  return digt;
		  }
	private:
		void init()
		{
			enable = true;
			T::_init(&s);
		}
		bool enable;
		typename T::_digest_date s;
	};



template<>
class digest_handle<FILESIZE_info, FILESIZE>
{
public:
	digest_handle():
	enable(false)
	{
		init();
	}
	~digest_handle(){}

	void update(const void *data, size_t len)
	{
		if(!enable)
		{
			init();
		}
		fsize += len;
	}
	string get_digest()
	{
		char digt[30] = {0};
#ifdef WIN32
        sprintf_s ( digt, 30, "%x", fsize );
#else // gun
        snprintf ( digt, 30, "%lld", fisize );
#endif      
		enable = false;
		return digt;
	}
private:
	void init()
	{
		enable = true;
		fsize = 0;
	}
	bool enable;
	long long fsize;
};
//////////////////////////////////////////////////////////////////////////
	template<typename T, unsigned int BUFSIZE = 1024>
	class calculate_digest
	{
	public:
		calculate_digest()
		{
		}
		~calculate_digest(){}
		int append_file(const string& _file)
		{
			FILE* from_fd;
			int bytes_read;
			int ret = 0;
			if ((from_fd=fopen(_file.c_str(), "rb"))==NULL) 
			{
				return -1;
			}

			while ((bytes_read=(int)fread(buf, sizeof(char), BUFSIZE, from_fd))) 
			{
				if (ferror(from_fd))
				{
					ret = -1;
					break;
				}
				else 
				{
					dig.update(buf, bytes_read);
				}
			}
			fclose(from_fd);
			return ret;
		}

		calculate_digest<T>& operator << (ifstream &o)
		{
			// string stream 
#ifdef TEXT_FILE
			stringstream _stream;
			_stream << o.rdbuf();
			dig.update(_stream.str().c_str(), _stream.str().size());
#else

			o.seekg (0, ios::end);
			long long total = o.tellg();
			o.seekg (0, ios::beg);
			if (total < 0)
			{
				cout << "calculate digest error " << endl;
				return *this;
			}
			else
			{
				//while(!in.eof()) 
				while(total > 0) 
				{
					int count = BUFSIZE < total ? BUFSIZE : total;
					o.read(buf, count);
					total -= count;
					dig.update(buf, count);
				}
			}
#endif // TEXT_FILE
			return *this;
		}

		string get_digest()
		{
			return dig.get_digest();
		}
	private:
		T dig;
#ifndef TEXT_FILE
		char buf[BUFSIZE];
#endif // TEXT_FILE
	};

	template<>
	calculate_digest<digest::size>& calculate_digest<digest::size>::operator << (ifstream &o)
	{
		o.seekg( 0L, ios::end ); 
		fstream::pos_type end_pos = o.tellg();
#ifdef WIN32        
		dig.update(NULL, end_pos.seekpos());
#else // gun
        dig.update(NULL, end_pos);
#endif
		return *this;
	}

	template<>
	calculate_digest<digest_handle<DIGESTNAME(FILESIZE), FILESIZE> >& calculate_digest<digest_handle<DIGESTNAME(FILESIZE), FILESIZE> >::operator << (ifstream &o)
	{
		o.seekg( 0L, ios::end ); 
		fstream::pos_type end_pos = o.tellg();
#ifdef WIN32
		dig.update(NULL, end_pos.seekpos());
#else // gun
        dig.update(NULL, end_pos);
#endif
		return *this;
	}

	typedef calculate_digest<digest_handle<DIGESTNAME(MD5), MD5> > digt_md5;
	typedef calculate_digest<digest_handle<DIGESTNAME(SHA1), SHA1> > digt_sha1;
	typedef calculate_digest<digest_handle<DIGESTNAME(SHA256), SHA256> > digt_sha256;
	typedef calculate_digest<digest_handle<DIGESTNAME(SHA512), SHA512> > digt_sha512;
	typedef calculate_digest<digest_handle<DIGESTNAME(FILESIZE), FILESIZE> > digt_size;
}

