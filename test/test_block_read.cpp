#include <bits/stdc++.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <fcntl.h>
using namespace std;

int fd;
char buf[100];

int main() {
    // 以可读写方式打开设备文件
    fd = open("/dev/kfifo_buf", O_RDWR);
    if (fd < 0) {
        printf("Open device file failed.\n");
        return -1;
    }

    // 一个写进程与两个读进程
    if (fork()) {
        if (fork()) {
            usleep(100000); // 等待读进程挂起
            write(fd, "a", 1);            
        } else {
            read(fd, buf, 1);
            exit(0);
        }
    } else {
        read(fd, buf, 1);
        exit(0);
    }

    wait(NULL);
    wait(NULL);
    close(fd);
    return 0;
}