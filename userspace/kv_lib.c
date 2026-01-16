#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "kv_lib.h"
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

int kv_del(int fd, const char *k)
{
    struct kv_key key = {};
    strcpy(key.key, k);
    key.key_len = strlen(k);
    if (ioctl(fd, KV_DEL, &key) == 0) {
        return 0;
    }
    return -1;
}

int kv_stat(int fd, kv_stat_t *output_stat)
{
    if (output_stat == NULL)
        return -1;

    struct kv_usage_stat stat;
    if (ioctl(fd, KV_STAT, &stat) != 0)
        return -1;

    output_stat->bucket_count = (unsigned long long) stat.bucket_count;
    output_stat->max_items = (unsigned long long) stat.max_items;
    output_stat->cur_items = (unsigned long long) stat.cur_items;
    output_stat->use_lru = (bool) stat.use_lru;
   
    return 0;
}

