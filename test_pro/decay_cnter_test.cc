// mds_recall_max_decay_rate 2.5
// mds_recall_warning_decay_rate 60
// recall_caps(ceph_clock_now(), g_conf->get_val<double>("mds_recall_warning_decay_rate")) // 60
// release_caps(ceph_clock_now(), g_conf->get_val<double>("mds_recall_warning_decay_rate")) // 60
// recall_caps_throttle(ceph_clock_now(), g_conf->get_val<double>("mds_recall_max_decay_rate")) // 2.5


// Session :
// DecayCounter recall_caps, release_caps
// DecayCounter recall_caps_throttle

// recall_caps.hit(ceph_clock_now(), count)
// recall_caps.get(ceph_clock_now());

// struct timespec ts;
// clock_gettime(CLOCK_REALTIME, &ts);
// ts->tv_sec
#include "unistd.h"
#include "DecayCounter.h"
#include <list>
int main(){
    int ti = false;
    cout << "ti : " << ti << endl;
    //std::list<DecayCounter*> col;
    //DecayCounter::generate_test_instances(col);
    struct timespec ts;
    uint64_t recall_caps = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    DecayCounter test_decay(ts, 5);
    clock_gettime(CLOCK_REALTIME, &ts);
    test_decay.hit(ts, 1);
    ts.tv_sec += 2;
    recall_caps = test_decay.get(ts);
    printf("clock_gettime : tv_sec=%ld, tv_nsec=%ld\n", ts.tv_sec, ts.tv_nsec);
    cout << "now caps : " << recall_caps << "( "<< (double)(utime_t)ts<< ")" << endl;
    //sleep(1);
    //clock_gettime(CLOCK_REALTIME, &ts);
    /*
    ts.tv_sec += 1;
    ts.tv_nsec += 0;
    recall_caps = test_decay.get(ts);
    printf("clock_gettime : tv_sec=%ld, tv_nsec=%ld\n", ts.tv_sec, ts.tv_nsec);
    cout << "now caps : " << recall_caps << "( "<< (double)(utime_t)ts<< ")" << endl;
     */
    ts.tv_sec += 2;
    cout << "now caps : " << test_decay.get(ts) << "( "<< (double)(utime_t)ts<< ")" << endl;   //
    test_decay.hit(ts, 10000);
    for(int i = 0; i< 20;i++){
        ts.tv_sec += 5;
        // test_decay.hit(ts, -1000);
        // ts.tv_nsec += 900000000; // ok
        cout << "now caps : " << test_decay.get(ts) << "  decay  " << "( "<< (double)(utime_t)ts<< ")" << endl;
    }
    return 0;
}
