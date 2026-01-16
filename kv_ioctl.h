#pragma once
#include <linux/ioctl.h>
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
typedef uint8_t  __u8;
typedef uint32_t __u32;
typedef uint64_t __u64;
#endif

#define KV_IOC_MAGIC 'k'
#define KV_MAX_KEY   64
#define KV_MAX_VAL   256

struct kv_key {
    char key[KV_MAX_KEY];
    __u32 key_len;
};

struct kv_pair {
    char key[KV_MAX_KEY];
    char value[KV_MAX_VAL];
    __u32 key_len;
    __u32 value_len;
};

struct kv_usage_stat {
    __u64 bucket_count;
    __u64 max_items;
    __u64 cur_items;
    __u8  use_lru;
};

#define KV_PUT _IOW(KV_IOC_MAGIC, 1, struct kv_pair)
#define KV_GET _IOWR(KV_IOC_MAGIC, 2, struct kv_pair)
#define KV_DEL _IOW(KV_IOC_MAGIC, 3, struct kv_key)
#define KV_STAT _IOR(KV_IOC_MAGIC, 4, struct kv_usage_stat)