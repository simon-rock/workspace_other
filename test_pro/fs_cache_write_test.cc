#include <iostream>
#include <string>
#include <string.h>
#include <cstdio>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

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
    cout << "start open file" << endl;
    // FILE* fp = fopen(filename.c_str(), "wb+");
    int fp = open(filename.c_str(), O_RDWR|O_CREAT);
    cout << "pause any key to continue!!" << endl;
    fgetc(stdin);

    struct timeval tv_begin, tv_end;
    long long write_timeuse = 0;

    gettimeofday(&tv_begin, NULL);
    cout << "start write file" << endl;
    /*
    if (fwrite(w_buf, g_block_size, 1, fp) <= 0) {
        cout << "fwrite error !!" << endl;
        cout << "err : " << errno << " " << strerror(errno) << endl;
        return -1;
        }*/
    volatile int len = 0;
    while(1){
        int n;
        if((n=write(fp, w_buf+len, (strlen(w_buf)-len)))== 0){
            break;
        }else if(n<0){
            cout << "write error !!" << endl;
            cout << "err : " << errno << " " << strerror(errno) << endl;
            return -1;
        }
        len+=n;
    }

    int ret = 0;
    /*
    cout << "start flush file" << endl;
    ret = fflush(fp);
    cout << "flush buffer to page, ret : " << ret << endl;
    if (ret < 0)
        cout << "err : " << errno << " " << strerror(errno) << endl;
    */
    cout << "after flush buffer, sleep 10s start " << endl;
    sleep(10);
    cout << "after flush buffer, sleep 10s end" << endl;

    //ret = fdatasync(fileno(fp));
    //ret = fsync(fileno(fp));
    //ret = fsync(fp);
    ret = fdatasync(fp);
    cout << "after fdatasync1 data, ret : " << ret << endl;
    if (ret < 0)
        cout << "err : " << errno << " " << strerror(errno) << endl;
    //gettimeofday(&tv_end, NULL);
    //write_timeuse += 1000000 * (tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec;
    close(fp);
    return 0;
}
// c库缓冲-----fflush---------〉内核缓冲--------fsync-----〉磁盘
// // https://blog.csdn.net/lhb0709/article/details/86077584
