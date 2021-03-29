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

    write(fd, buf, 100); // 写满缓冲区

    // 一个读进程与两个写进程
    if (fork()) {
        if (fork()) {
            usleep(100000); // 等待写进程挂起
            read(fd, buf, 1);            
        } else {
            write(fd, "a", 1);
            exit(0);
        }
    } else {
        write(fd, "a", 1);
        exit(0);
    }

    wait(NULL);
    wait(NULL);
    read(fd, buf, 100); // 清空缓冲区
    close(fd);
    return 0;
}