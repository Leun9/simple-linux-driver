#include <bits/stdc++.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <fcntl.h>
using namespace std;

int fd, ret;
char buf[100];

int main() {
    for (int i = 0; i <= 1; i++) {
        // 以可读写方式打开设备文件
        fd = open("/dev/kfifo_buf", O_RDWR | (i * O_NONBLOCK));
        if (fd < 0) {
            printf("Open device file failed.\n");
            return -1;
        } else 
            printf("\nnonblock = %d:\n", i);
        
        read(fd, buf, 100);

        write(fd, "abcdefghijklmnopqrstuvwxyz", 26);

        ret = read(fd, buf, 13);
        buf[ret] = 0;
        printf("%s\n", buf);

        write(fd, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);

        ret = read(fd, buf, 100);
        buf[ret] = 0;
        printf("%s\n", buf);

        close(fd);
    }
    return 0;
}