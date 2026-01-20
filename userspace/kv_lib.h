#ifndef KV_LIB_H
#define KV_LIB_H
#include <stdint.h>

#define KV_IOC_MAGIC 'k'
#define KV_MAX_KEY   64
#define KV_MAX_VAL   256

typedef uint8_t kv_u8_t;
typedef uint32_t kv_u32_t;
typedef uint64_t kv_u64_t;

struct kv_key {
    char data[KV_MAX_KEY];
    kv_u32_t len;
};

struct kv_value {
    char data[KV_MAX_VAL];
    kv_u32_t len;
};

struct kv_pair {
    struct kv_key key;
    struct kv_value value;
};

struct kv_usage_stat {
    kv_u64_t bucket_count;
    kv_u64_t max_items;
    kv_u64_t cur_items;
    kv_u8_t  use_lru;
};

int kv_open(void);
int kv_close(int fd);

int kv_put(int fd, const struct kv_pair *pair);
int kv_get(int fd, struct kv_pair *pair);
int kv_del(int fd, const struct kv_key *key);
int kv_stat(int fd, struct kv_usage_stat *stat_out);
const char *kv_err_msg(int err);

#define KV_PUT _IOW(KV_IOC_MAGIC, 1, struct kv_pair)
#define KV_GET _IOWR(KV_IOC_MAGIC, 2, struct kv_pair)
#define KV_DEL _IOW(KV_IOC_MAGIC, 3, struct kv_key)
#define KV_STAT _IOR(KV_IOC_MAGIC, 4, struct kv_usage_stat)

#endif // KV_IOCTL_H
