#include <stdio.h>
#include <string.h>
#include "libkv.h"

int main(void)
{
    int fd = kv_open("/dev/kvstore");
    if (fd < 0) {
        perror("kv_open");
        return 1;
    }

    if (kv_put(fd, "hello", "world") < 0) {
        perror("kv_put");
    }

    char buf[128] = {0};
    if (kv_get(fd, "hello", buf, sizeof(buf)) == 0) {
        printf("GET: hello -> %s\n", buf);
    } else {
        perror("kv_get");
    }

    if (kv_del(fd, "hello") == 0) {
        printf("Key 'hello' deleted successfully\n");
    } else {
        perror("kv_del");
    }

    kv_close(fd);
    return 0;
}
