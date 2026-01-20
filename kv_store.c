#include "kv_store.h"

inline static u32 kv_key_hash(struct kv_key *key)
{
    return jhash(key->data, key->len, 0);
}

inline static int kv_key_equal(struct kv_key *key1, struct kv_key *key2)
{
    return key1->len == key2->len && memcmp(key1->data, key2->data, key1->len) == 0;
}

inline static void kv_item_init(struct kv_item *item, struct kv_pair *p)
{
    memcpy(item->key.data, p->key.data, p->key.len);
    memcpy(item->value.data, p->value.data, p->value.len);
    item->key.len = p->key.len;
    item->value.len = p->value.len;
    INIT_LIST_HEAD(&item->lru_node);
}

inline static void kv_item_set_value(struct kv_item *item, struct kv_pair *p) 
{
    memcpy(item->value.data, p->value.data, p->value.len);
    item->value.len = p->value.len;
}

int kv_store_init(struct kv_store *s, size_t buckets, size_t max_items, bool lru)
{
    s->bucket_count = buckets;
    s->max_items = max_items;
    s->use_lru = lru;
    atomic_set(&s->curr_items, 0);

    s->buckets = kmalloc_array(buckets, sizeof(*s->buckets), GFP_KERNEL);
    if (!s->buckets)
        return -ENOMEM;

    for (size_t i = 0; i < buckets; i++) {
        INIT_HLIST_HEAD(&s->buckets[i].head);
    }

    mutex_init(&s->lock);
    lru_init(&s->lru);

    return 0;
}

void kv_store_destroy(struct kv_store *s)
{
    size_t i;
    struct kv_item *item;
    struct hlist_node *tmp;

    mutex_lock(&s->lock);
    for (i = 0; i < s->bucket_count; i++) {
        hlist_for_each_entry_safe(item, tmp, &s->buckets[i].head, hnode) {
            hlist_del(&item->hnode);
            kfree(item);
        }
    }

    kfree(s->buckets);
    mutex_unlock(&s->lock);
}

static struct kv_item *kv_bucket_find_item(struct kv_bucket *b, struct kv_key *key) 
{
    struct kv_item *item = NULL;
    hlist_for_each_entry(item, &b->head, hnode) {
        if (kv_key_equal(&item->key, key)) {        
            break;
        }
    }

    return item;
}

static struct kv_item *kv_check_and_evict_item(struct kv_store *s)
{
    struct kv_item *victim;

    if (s->use_lru && atomic_read(&s->curr_items) >= s->max_items) {
        victim = lru_evict(&s->lru);
        if (victim) {
            hlist_del_init(&victim->hnode);
        }
    } 

    return victim;
}

int kv_put(struct kv_store *s, struct kv_pair *p)
{
    if (s == NULL || p == NULL || p->key.len > KV_MAX_KEY 
        || p->value.len > KV_MAX_VAL) 
        return -EINVAL;

    u32 hash = kv_key_hash(&p->key) % s->bucket_count;
    struct kv_bucket *b = &s->buckets[hash];
    struct kv_item *item;

    mutex_lock(&s->lock);
    item = kv_bucket_find_item(b, &p->key);
    if (item) {
        kv_item_set_value(item, p);
        lru_touch(&s->lru, item);
        mutex_unlock(&s->lock);
        return 0;
    } 

    item = kv_check_and_evict_item(s);
    if (!item) {
        item = kmalloc(sizeof(*item), GFP_KERNEL);
        if (!item) {
            mutex_unlock(&s->lock);
            return -ENOMEM;
        }
        atomic_inc(&s->curr_items);
    }

    kv_item_init(item, p);
    hlist_add_head(&item->hnode, &b->head);
    if (s->use_lru) {
        lru_touch(&s->lru, item);
    }
    mutex_unlock(&s->lock);

    return 0;
}

int kv_get(struct kv_store *s, struct kv_pair *p)
{
    if (s == NULL || p == NULL || p->key.len > KV_MAX_KEY)
        return -EINVAL;

    u32 hash = kv_key_hash(&p->key) % s->bucket_count;
    struct kv_bucket *b = &s->buckets[hash];
    struct kv_item *item;
    
    mutex_lock(&s->lock);
    item = kv_bucket_find_item(b, &p->key);
    if (!item) {
        mutex_unlock(&s->lock);
        return -ENOENT;
    }

    memcpy(p->value.data, item->value.data, item->value.len);
    p->value.len = item->value.len;
    if (s->use_lru) {
        lru_touch(&s->lru, item);
    }
    mutex_unlock(&s->lock);

    return 0;
}

int kv_del(struct kv_store *s, struct kv_key *k)
{
    if (s == NULL || k == NULL || k->len > KV_MAX_KEY)
        return -EINVAL;

    u32 hash = kv_key_hash(k) % s->bucket_count;
    struct kv_bucket *b = &s->buckets[hash];
    struct kv_item *item;

    mutex_lock(&s->lock);
    item = kv_bucket_find_item(b, k);
    if (!item) {
        mutex_unlock(&s->lock);
        return -ENOENT;
    }

    hlist_del(&item->hnode);
    lru_remove(&s->lru, item);
    kfree(item);
    atomic_dec(&s->curr_items);
    mutex_unlock(&s->lock);

    return 0;
}

int kv_stat(struct kv_store *s, struct kv_usage_stat *stat)
{
    if (s == NULL || stat == NULL)
        return -EINVAL;
    
    stat->bucket_count = s->bucket_count;
    stat->max_items = s->max_items;
    stat->use_lru = s->use_lru;
    stat->curr_items = atomic_read(&s->curr_items);

    return 0;
}
