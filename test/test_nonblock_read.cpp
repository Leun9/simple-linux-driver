#include <bits/stdc++.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <fcntl.h>
using namespace std;

int fd;
char buf[100];

int main() {
    // 以可读写方式打开设备文件
    fd = open("/dev/kfifo_buf", O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        printf("Open device file failed.\n");
        return -1;
    }
    
    read(fd, buf, 1);

    close(fd);
    return 0;
}