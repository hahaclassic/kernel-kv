#pragma once
#include <linux/ioctl.h>

#define KV_IOC_MAGIC 'k'
#define KV_MAX_KEY   64
#define KV_MAX_VAL   256

struct kv_pair {
    char key[KV_MAX_KEY];
    char value[KV_MAX_VAL];
    size_t value_len;
};

#define KV_PUT _IOW(KV_IOC_MAGIC, 1, struct kv_pair)
#define KV_GET _IOWR(KV_IOC_MAGIC, 2, struct kv_pair)
#define KV_DEL _IOW(KV_IOC_MAGIC, 3, struct kv_pair)