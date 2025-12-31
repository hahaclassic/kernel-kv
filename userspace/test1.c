#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "../kv_ioctl.h"

int main(void)
{
    int fd = open("/dev/kvstore", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct kv_pair p = {};

    strcpy(p.key, "hello");
    strcpy(p.value, "world");
    p.value_len = strlen(p.value) + 1;

    if (ioctl(fd, KV_PUT, &p) < 0)
        perror("KV_PUT");

    memset(p.value, 0, sizeof(p.value));

    if (ioctl(fd, KV_GET, &p) == 0)
        printf("GET: %s -> %s\n", p.key, p.value);
    else
        perror("KV_GET");

    close(fd);
    return 0;
}
