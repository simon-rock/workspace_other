#include <iostream>
#include <string>
#include <string.h>
#include <cstdio>
#include <sys/time.h>
#include <unistd.h>    
#include <errno.h>

using namespace std;

int main(int argc, char** argv){
    string filename;
    char def_buf[60] = "fs_cache_test";
    char w_buf[60] = {0};
    if(argc > 1)
        filename = argv[1];
    if (argc > 2){
        memcpy(w_buf, argv[2], strlen(argv[2]));
    }
    else
        memcpy(w_buf, def_buf, strlen(def_buf));
    w_buf[59] = 0;
    
    int g_block_size = strlen(w_buf);
    FILE* fp = fopen(filename.c_str(), "wb+");
    struct timeval tv_begin, tv_end;
    long long write_timeuse = 0;
    
    gettimeofday(&tv_begin, NULL);
    if (fwrite(w_buf, g_block_size, 1, fp) <= 0) {
        cout << "fwrite error !!" << endl;
        cout << "err : " << errno << " " << strerror(errno) << endl;
        return -1;
    }
    cout << "after write buffer, sleep 5s" << endl;
    sleep(5);

    int ret = fflush(fp);
    cout << "after flush buffer to page, sleep 5s : " << ret << endl;
    if (ret < 0)
        cout << "err : " << errno << " " << strerror(errno) << endl;
    sleep(5);
    ret = fdatasync(fileno(fp));
    if (ret < 0)
        cout << "err : " << errno << " " << strerror(errno) << endl;
    cout << "after fync data, sleep 5s : " << ret << endl;
    gettimeofday(&tv_end, NULL);
    write_timeuse += 1000000 * (tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec;
    return 0;
}
// c库缓冲-----fflush---------〉内核缓冲--------fsync-----〉磁盘
// https://blog.csdn.net/lhb0709/article/details/86077584
