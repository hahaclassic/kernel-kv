#ifndef KV_STORE_H
#define KV_STORE_H

#include <linux/hashtable.h>
#include <linux/mutex.h>
#include <linux/jhash.h>
#include <linux/string.h>
#include <linux/slab.h>
#include "kv_types.h"
#include "kv_lru.h"

#define BUCKETS_DEFAULT 128
#define MAX_ITEMS_DEFAULT 256
#define USE_LRU_DEFAULT 0

int kv_store_init(struct kv_store *s, size_t buckets, size_t max_items, bool lru);
void kv_store_destroy(struct kv_store *s);

int kv_put(struct kv_store *s, struct kv_pair *p);
int kv_get(struct kv_store *s, struct kv_pair *p);
int kv_del(struct kv_store *s, struct kv_key *k);
int kv_stat(struct kv_store *store, struct kv_usage_stat *stat);

#endif // KV_STORE_H