#ifndef _STIME_H_
#define _STIME_H_
#define STATISTICS

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
namespace statistics
{
    #if defined (WIN32)
    #include <windows.h>
    #define time(X) GetTickCount()
    #define UNIT "ms"
    #else
    #include <time.h>
    #include <sys/time.h>
    #define UNIT "us"
    #endif // WIN32
    static std::ofstream _df;
    static bool init = false;
    class time_cal
    {
    public:
        time_cal():elapse(0), vaild(false){}
        ~time_cal(){}
        void start(char* file, char* fun)
        {
            ss.str("");
            ss << "[" << fun << "][" << file << "] -- ";
            //elapse = time(NULL);
            struct timeval s_start;
            gettimeofday(&s_start, NULL);
            elapse = s_start.tv_sec * 1000000LL + s_start.tv_usec;
            vaild = false;
        }
        void end()
        {
            struct timeval s_start;
            gettimeofday(&s_start, NULL);
            elapse = s_start.tv_sec * 1000000LL + s_start.tv_usec - elapse;
            //elapse = time(NULL) - elapse;
            vaild = true;
        }
        std::string output()
        {
            if (vaild)
                ss << "elapse time : " << elapse << UNIT << std::endl;
            return ss.str();
        }
    private:
        long long elapse;
        bool vaild;
        std::stringstream ss;
    };

    class memory
    {

    };

    static time_cal def_time;
    template<typename T>
    class scoped_res
    {
    public:
        scoped_res(char* file, char* fun, T* t, bool dcal = false, std::ostream* o = NULL)
            :_m(NULL), _o(NULL), _clean(false), _dcal(dcal)
        {
            _m = t;
            _m->start(file, fun);
            if (!init)
            {
                if (o)
                {
                    _o = o;
                    _clean = true;
                }
                else
                    _o = &(std::cout);
            }
            else
            {
                _o = &_df;
            }
//            (*_o) << "[" << file << "][" << fun << "] -- ";
        }
        ~scoped_res()
        {
            _m->end();
            (*_o) << _m->output();
            (*_o).flush();
            if(_clean) delete _o;
            if(_dcal) delete _m;
        }
        static void init_scoped_log()
        {
            if (!init)
            {
                _df.open( "default_log.log", std::ios::app );
                if (_df)
                {
                    init = true;
                }
            }
        }
    private:
        T* _m;
        std::ostream* _o;
        bool _clean;
        bool _dcal;
    };

#define STATISTICSTIME_NAME(LOG, NAME) statistics::scoped_res<statistics::time_cal> __FUNCTION__##stat (__FILE__, NAME, new statistics::time_cal, true, new std::ofstream(LOG, std::ios::app) );
#define STATISTICSTIMECOUT_NAME(NAME) statistics::scoped_res<statistics::time_cal> __FUNCTION__##stat (__FILE__, NAME, new statistics::time_cal, true);

#define STATISTICSTIME(LOG) STATISTICSTIME_NAME(LOG, __FUNCDNAME__)
#define STATISTICSTIMECOUT STATISTICSTIMECOUT_NAME(__FUNCDNAME__)
//

// use default time
#define STATISTICSTIMEDEFSINGLETHREAD statistics::scoped_res<statistics::time_cal>::init_scoped_log();statistics::scoped_res<statistics::time_cal> __FUNCTION__##stat (__FILE__, __FUNCDNAME__, &statistics::def_time);
#define STATISTICSTIMECOUTDEFSINGLETHREAD statistics::scoped_res<statistics::time_cal> __FUNCTION__##stat (__FILE__, __FUNCDNAME__, &statistics::def_time);
}
#endif //_STIME_H_
