#pragma once

/* forward declarations */
struct kv_store;
struct kv_item;

/*
 * LRU helpers
 */

void lru_touch(struct kv_store *s,
               struct kv_item *item);

/*
 * Returns victim item already removed from LRU list.
 */
struct kv_item *lru_evict(struct kv_store *s);
