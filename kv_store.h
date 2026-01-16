#pragma once
#include <linux/hashtable.h>
#include <linux/mutex.h>
#include <linux/jhash.h>
#include <linux/string.h>
#include <linux/slab.h>
#include "kv_ioctl.h"
#include "kv_lru.h"

struct kv_item {
    struct hlist_node hnode;
    struct list_head lru_node;
    char key[KV_MAX_KEY];
    char value[KV_MAX_VAL];
    size_t value_len;
};

struct kv_bucket {
    struct hlist_head head;
    struct mutex lock;
};

struct kv_store {
    struct kv_bucket *buckets;
    size_t bucket_count;
    size_t max_items;
    atomic_t cur_items;
    bool use_lru;

    struct list_head lru_list;
    spinlock_t lru_lock;
};

int kv_store_init(struct kv_store *s, size_t buckets, size_t max_items, bool lru);
void kv_store_destroy(struct kv_store *s);

int kv_put(struct kv_store *s, struct kv_pair *p);
int kv_get(struct kv_store *s, struct kv_pair *p);
int kv_del(struct kv_store *s, struct kv_key *k);
int kv_stat(struct kv_store *store, struct kv_usage_stat *stat);
