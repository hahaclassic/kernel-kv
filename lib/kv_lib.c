#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include "kv_lib.h"

int kv_open(void)
{
    return open("/dev/kvstore", O_RDWR);
}

int kv_close(int fd)
{
    return close(fd);
}

int kv_put(int fd, const struct kv_pair *pair) {
    if (ioctl(fd, KV_PUT, pair) < 0)
        return -errno;
    return 0;
}

int kv_get(int fd, struct kv_pair *pair) {
    if (ioctl(fd, KV_GET, pair) < 0)
        return -errno;
    return 0;
}

int kv_del(int fd, const struct kv_key *key) {
    if (ioctl(fd, KV_DEL, key) < 0)
        return -errno;
    return 0;
}

int kv_stat(int fd, struct kv_usage_stat *stat) {
    if (ioctl(fd, KV_STAT, stat) < 0)
        return -errno;
    return 0;
}

const char *kv_err_msg(int err)
{
    switch (err) {
        case 0:
            return "Success";
        case -ENOENT:
            return "Key not found";
        case -EINVAL:
            return "Invalid argument";
        case -ENOSPC:
            return "No space left in store";
        case -ENOMEM:
            return "Out of memory";
        case -EFAULT:
            return "Bad user pointer";
        default:
            return "Unknown error";
    }
}
