#ifndef KV_TYPES_H
#define KV_TYPES_H

#include <linux/types.h>
#include <linux/mutex.h>

#define KV_MAX_KEY   64
#define KV_MAX_VAL   256

typedef __u8 kv_u8;
typedef __u32 kv_u32;
typedef __u64 kv_u64;

struct kv_key {
    char data[KV_MAX_KEY];
    kv_u32 len;
};

struct kv_value {
    char data[KV_MAX_VAL];
    kv_u32 len;
};

struct kv_pair {
    struct kv_key key;
    struct kv_value value;
};

struct kv_usage_stat {
    kv_u64 bucket_count;
    kv_u64 max_items;
    kv_u64 curr_items;
    kv_u8  use_lru;
};

struct kv_item {
    struct hlist_node hnode;
    struct list_head lru_node;
    struct kv_key key;
    struct kv_value value;
};

struct kv_bucket {
    struct hlist_head head;
};

struct kv_lru {
    struct list_head head;
};

struct kv_store {
    struct kv_bucket *buckets;
    struct kv_lru lru;
    struct mutex lock;

    kv_u8 use_lru;
    kv_u64 bucket_count;
    kv_u64 max_items;
    atomic_t curr_items;
};

#endif // KV_TYPES_H