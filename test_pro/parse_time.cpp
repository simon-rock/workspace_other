#include <iostream>
#include <string>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include<stdio.h>
#include <string.h>
#include <sstream>
#include <iomanip>
using namespace std;

typedef int int32_t;
typedef unsigned int __u32;
typedef unsigned long uint64_t;
#include "/home/yu/workspace/ceph_build/ceph/src/include/timegm.h"
// #include "ceph-12.2.10/src/common/strtol.h"
int strict_strtol(const char *ino, int base, std::string *err){
    unsigned long long iino = 0;
    std::stringstream conv;
    conv << ino;
    conv >> iino;
    printf("convert to hexadecimal ino  %s => %d \n", ino, iino);
    return iino;
}
class utime_t {
public:
  struct {
    __u32 tv_sec, tv_nsec;
  } tv;

 public:
  bool is_zero() const {
    return (tv.tv_sec == 0) && (tv.tv_nsec == 0);
  }
  void normalize() {
    if (tv.tv_nsec > 1000000000ul) {
      tv.tv_sec += tv.tv_nsec / (1000000000ul);
      tv.tv_nsec %= 1000000000ul;
    }
  }

    // accessors
    time_t        sec()  const { return tv.tv_sec; }
    long          usec() const { return tv.tv_nsec/1000; }
    int           nsec() const { return tv.tv_nsec; }
  // cons
  utime_t() { tv.tv_sec = 0; tv.tv_nsec = 0; }
  utime_t(time_t s, int n) { tv.tv_sec = s; tv.tv_nsec = n; normalize(); }

      std::ostream& localtime(std::ostream& out) const {
    out.setf(std::ios::right);
    char oldfill = out.fill();
    out.fill('0');
    if (sec() < ((time_t)(60*60*24*365*10))) {
      // raw seconds.  this looks like a relative time.
      out << (long)sec() << "." << std::setw(6) << usec();
    } else {
      // this looks like an absolute time.
      //  conform to http://en.wikipedia.org/wiki/ISO_8601
      struct tm bdt;
      time_t tt = sec();
      localtime_r(&tt, &bdt);
      out << std::setw(4) << (bdt.tm_year+1900)  // 2007 -> '07'
	  << '-' << std::setw(2) << (bdt.tm_mon+1)
	  << '-' << std::setw(2) << bdt.tm_mday
	  << 'T'
	  << std::setw(2) << bdt.tm_hour
	  << ':' << std::setw(2) << bdt.tm_min
	  << ':' << std::setw(2) << bdt.tm_sec;
      out << "." << std::setw(6) << usec();
      char buf[32] = { 0 };
      strftime(buf, sizeof(buf), "%z", &bdt);
      out << buf;
    }
    out.fill(oldfill);
    out.unsetf(std::ios::right);
    return out;
  }
    
    ostream& localtime_old(ostream& out) const {
    out.setf(std::ios::right);
    char oldfill = out.fill();
    out.fill('0');
    if (sec() < ((time_t)(60*60*24*365*10))) {
      // raw seconds.  this looks like a relative time.
      out << (long)sec() << "." << std::setw(6) << usec();
    } else {
      // localtime.  this looks like an absolute time.
      //  aim for http://en.wikipedia.org/wiki/ISO_8601
      struct tm bdt;
      time_t tt = sec();
      localtime_r(&tt, &bdt);
      out << std::setw(4) << (bdt.tm_year+1900)  // 2007 -> '07'
	  << '-' << std::setw(2) << (bdt.tm_mon+1)
	  << '-' << std::setw(2) << bdt.tm_mday
	  << ' '
	  << std::setw(2) << bdt.tm_hour
	  << ':' << std::setw(2) << bdt.tm_min
	  << ':' << std::setw(2) << bdt.tm_sec;
      out << "." << std::setw(6) << usec();
      //out << '_' << bdt.tm_zone;
    }
    out.fill(oldfill);
    out.unsetf(std::ios::right);
    return out;
  }

    static int parse_date(const std::string& date, uint64_t *epoch, uint64_t *nsec,
                        std::string *out_date=nullptr,
			std::string *out_time=nullptr) {
    struct tm tm;
    memset(&tm, 0, sizeof(tm));

    if (nsec)
      *nsec = 0;

    const char *p = strptime(date.c_str(), "%Y-%m-%d", &tm);
    if (p) {
      if (*p == ' ' || *p == 'T') {
	p++;
	// strptime doesn't understand fractional/decimal seconds, and
	// it also only takes format chars or literals, so we have to
	// get creative.
	char fmt[32] = {0};
	strncpy(fmt, p, sizeof(fmt) - 1);
	fmt[0] = '%';
	fmt[1] = 'H';
	fmt[2] = ':';
	fmt[3] = '%';
	fmt[4] = 'M';
	fmt[6] = '%';
	fmt[7] = 'S';
	const char *subsec = 0;
	char *q = fmt + 8;
	if (*q == '.') {
	  ++q;
	  subsec = p + 9;
	  q = fmt + 9;
	  while (*q && isdigit(*q)) {
	    ++q;
	  }
	}
	// look for tz...
	if (*q == '-' || *q == '+') {
	  *q = '%';
	  *(q+1) = 'z';
	  *(q+2) = 0;
	}
	p = strptime(p, fmt, &tm);
	if (!p) {
	  return -EINVAL;
	}
        if (nsec && subsec) {
          unsigned i;
          char buf[10]; /* 9 digit + null termination */
          for (i = 0; (i < sizeof(buf) - 1) && isdigit(*subsec); ++i, ++subsec) {
            buf[i] = *subsec;
          }
          for (; i < sizeof(buf) - 1; ++i) {
            buf[i] = '0';
          }
          buf[i] = '\0';
	  std::string err;
          *nsec = (uint64_t)strict_strtol(buf, 10, &err);
          if (!err.empty()) {
            return -EINVAL;
          }
        }
      }
    } else {
      int sec, usec;
      int r = sscanf(date.c_str(), "%d.%d", &sec, &usec);
      if (r != 2) {
        return -EINVAL;
      }

      time_t tt = sec;
      gmtime_r(&tt, &tm);

      if (nsec) {
        *nsec = (uint64_t)usec * 1000;
      }
    }

    // apply the tm_gmtoff manually below, since none of mktime,
    // gmtime, and localtime seem to do it.  zero it out here just in
    // case some other libc *does* apply it.  :(

    // get gmtoff .because tm.tm_gmtoff is always 0.
    
    time_t timep;
    struct tm tmptm;
    time(&timep);
    localtime_r(&timep, &tmptm);
    auto gmtoff_ex = tmptm.tm_gmtoff;
    
    cout << " tm.tm_gmtoff : " << tm.tm_gmtoff << " "<< gmtoff_ex << endl;
    
    auto gmtoff = tm.tm_gmtoff;
    tm.tm_gmtoff = 0;

    //time_t t = internal_timegm(&tm);
    time_t t = mktime(&tm);
    if (epoch)
      *epoch = (uint64_t)t;

    *epoch -= gmtoff;

    if (out_date) {
      char buf[32];
      strftime(buf, sizeof(buf), "%F", &tm);
      *out_date = buf;
    }
    if (out_time) {
      char buf[32];
      strftime(buf, sizeof(buf), "%T", &tm);
      *out_time = buf;
    }

    return 0;
  }
    
  bool parse(const std::string& s) {
    uint64_t epoch, nsec;
    int r = parse_date(s, &epoch, &nsec);
    if (r < 0) {
      return false;
    }
    *this = utime_t(epoch, nsec);
    return true;
  }
};

inline std::ostream& operator<<(std::ostream& out, const utime_t& t)
{
  return t.localtime(out);
}
static int parse_date_old(const string& date, uint64_t *epoch, uint64_t *nsec,
                        string *out_date=NULL, string *out_time=NULL)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));

    if (nsec)
      *nsec = 0;

    const char *p = strptime(date.c_str(), "%Y-%m-%d", &tm);
    if (p) {
      if (*p == ' ') {
	p++;
	p = strptime(p, " %H:%M:%S", &tm);
	if (!p)
	  return -EINVAL;
        if (nsec && *p == '.') {
          ++p;
          unsigned i;
          char buf[10]; /* 9 digit + null termination */
          for (i = 0; (i < sizeof(buf) - 1) && isdigit(*p); ++i, ++p) {
            buf[i] = *p;
          }
          for (; i < sizeof(buf) - 1; ++i) {
            buf[i] = '0';
          }
          buf[i] = '\0';
          string err;
          *nsec = (uint64_t)strict_strtol(buf, 10, &err);
          if (!err.empty()) {
            return -EINVAL;
          }
        }
      }
    } else {
      int sec, usec;
      int r = sscanf(date.c_str(), "%d.%d", &sec, &usec);
      if (r != 2) {
        return -EINVAL;
      }

      time_t tt = sec;
      gmtime_r(&tt, &tm);
      cout << "yes simon " << endl;
      if (nsec) {
        *nsec = (uint64_t)usec * 1000;
      }
    }

    // add by simon
    cout << asctime(&tm) << endl;
    time_t timep;
    struct tm tmptm;
    time(&timep);
    localtime_r(&timep, &tmptm);
    // tm.tm_zone = tmptm.tm_zone;       // simon, type is const char *
    // time_t t = mktime(&tm);           // simon, other reason??

    long gmtoff = tmptm.tm_gmtoff;       // patch from master
    tm.tm_gmtoff = 0;
    // cout << " kkk " << tm.tm_gmtoff << tm.tm_zone << endl; // tm_zone is null
    time_t t = internal_timegm(&tm);
    
    if (epoch)
      *epoch = (uint64_t)t;

    *epoch -= gmtoff;                   // patch from master

    if (out_date) {
      char buf[32];
      strftime(buf, sizeof(buf), "%F", &tm);
      *out_date = buf;
    }
    if (out_time) {
      char buf[32];
      strftime(buf, sizeof(buf), "%T", &tm);
      *out_time = buf;
    }

    return 0;
}


int main(){
    utime_t val;
    //string s = "2020-03-18 17:08:42.093386_CST";
    //string s = "2020-03-18 17:08:42.093386";
    string s = "2019-04-24 16:06:53.039991-0100";
    uint64_t epoch;
    uint64_t nsec;
    string s1, s2;
    //int r = parse_date(s, &epoch, &nsec, &s1, &s2);
    int r = utime_t::parse_date(s, &epoch, &nsec, &s1, &s2);
    if (r == 0)
        val = utime_t(epoch, nsec);
    else
        cout << "parse failed" << endl;
    cout << s << " after parse : " << val << endl;
    cout << s1 << " " << s2 << endl;

    { // test gmtime_r localtime_r mktime

        cout << "--test1--" << std::endl;
        time_t timep;
        struct tm tm,tm2;
        time(&timep);
        cout  << timep << endl;
        utime_t testval = utime_t(timep, 0);
        cout  << testval << endl;
        gmtime_r(&timep, &tm);
        cout << "gtime : " << mktime(&tm) << " " << asctime(&tm) << tm.tm_zone << endl;
        localtime_r(&timep, &tm2);
        cout << "ltime : " << mktime(&tm2) << " " << asctime(&tm2) << tm2.tm_zone << endl;
    }
    {
        cout << "--test2--" << std::endl;
        struct tm timeinfo;
        char buffer[80];
        memset( &timeinfo, 0, sizeof(struct tm));
        strptime("2001-11-12 18:31:01", "%Y-%m-%d %H:%M:%S",  &timeinfo);
        cout << "33" << timeinfo.tm_zone << endl;
    }
    return 0;
}
