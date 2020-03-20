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

    ostream& localtime(ostream& out) const {
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
};

inline std::ostream& operator<<(std::ostream& out, const utime_t& t)
{
  return t.localtime(out);
}
static int parse_date(const string& date, uint64_t *epoch, uint64_t *nsec,
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
    string s = "2020-03-18 17:08:42.093386";
    uint64_t epoch;
    uint64_t nsec;
    string s1, s2;
    int r = parse_date(s, &epoch, &nsec, &s1, &s2);
    if (r == 0)
        val = utime_t(epoch, nsec);
    else
        cout << "parse failed" << endl;
    cout << s << " after parse : " << val << endl;
    cout << s1 << " " << s2 << endl;

    { // test gmtime_r localtime_r mktime
        time_t timep;
        struct tm tm,tm2;
        time(&timep);
        cout  << timep << endl;
        utime_t testval = utime_t(timep, 0);
        cout  << testval << endl;
        gmtime_r(&timep, &tm);
        cout << mktime(&tm) << " " << asctime(&tm) << tm.tm_zone << endl;
        localtime_r(&timep, &tm2);
        cout << mktime(&tm2) << " " << asctime(&tm2) << tm.tm_zone << endl;
    }
    {
        struct tm timeinfo;
        char buffer[80];
        memset( &timeinfo, 0, sizeof(struct tm));
        strptime("2001-11-12 18:31:01", "%Y-%m-%d %H:%M:%S",  &timeinfo);
        cout << "33" << timeinfo.tm_zone << endl;
    }
    return 0;
}
