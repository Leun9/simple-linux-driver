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

    memset(buf, 'a', 100);
    write(fd, buf, 100); // 写满缓冲区
    write(fd, buf, 100);

    read(fd, buf, 100); // 清空缓冲区
    close(fd);
    return 0;
}