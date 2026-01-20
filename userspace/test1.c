#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "kv_lib.h"

int main(void)
{
    int fd = open("/dev/kvstore", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct kv_pair p = {};

    strcpy(p.key.data, "hello\0");
    p.key.len = strlen(p.key.data) + 1;
    strcpy(p.value.data, "world\0");
    p.value.len = strlen(p.value.data) + 1;

    printf("KV_PUT: %s -> %s", p.key.data, p.value.data);
    if (ioctl(fd, KV_PUT, &p) < 0)
        perror("KV_PUT");

    if (ioctl(fd, KV_GET, &p) == 0)
        printf("GET: %s -> %s\n", p.key.data, p.value.data);
    else
        perror("KV_GET");

    close(fd);
    return 0;
}
