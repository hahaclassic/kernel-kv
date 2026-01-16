#ifndef KV_LIB_H
#define KV_LIB_H

#include <stdbool.h>

typedef struct kv_stat_t {
    unsigned long long bucket_count;
    unsigned long long max_items;
    unsigned long long cur_items;
    bool use_lru;
} kv_stat_t;

int kv_open(void);
int kv_put(int fd, const char *key, const char *val);
int kv_get(int fd, const char *key, char *val_out);
int kv_del(int fd, const char *key);
int kv_stat(int fd, kv_stat_t *stat);
int kv_close(int fd);

#endif