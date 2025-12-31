#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "libkv.h"
#include "../kv_ioctl.h"

int kv_open(void)
{
    return open("/dev/kvstore", O_RDWR);
}

int kv_put(int fd, const char *k, const char *v)
{
    struct kv_pair p = {};
    strcpy(p.key, k);
    strcpy(p.value, v);
    p.value_len = strlen(v) + 1;
    return ioctl(fd, KV_PUT, &p);
}

int kv_get(int fd, const char *k, char *out)
{
    struct kv_pair p = {};
    strcpy(p.key, k);
    if (ioctl(fd, KV_GET, &p) == 0) {
        strcpy(out, p.value);
        return 0;
    }
    return -1;
}
