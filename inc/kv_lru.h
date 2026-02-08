#ifndef KV_LRU_H
#define KV_LRU_H

#include "kv_types.h"

void lru_init(struct kv_lru *lru);
void lru_touch(struct kv_lru *lru, struct kv_item *item);
void lru_remove(struct kv_lru *lru, struct kv_item *item);
struct kv_item *lru_evict(struct kv_lru *lru);

#endif // KV_LRU_H