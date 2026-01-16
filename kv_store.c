#include "kv_store.h"

static u32 hash_key(const char *key)
{
    return jhash(key, strlen(key), 0);
}

int kv_store_init(struct kv_store *s, size_t buckets, size_t max_items, bool lru)
{
    size_t i;

    s->bucket_count = buckets;
    s->max_items = max_items;
    s->use_lru = lru;
    atomic_set(&s->cur_items, 0);

    s->buckets = kmalloc_array(buckets, sizeof(*s->buckets), GFP_KERNEL);
    if (!s->buckets)
        return -ENOMEM;

    for (i = 0; i < buckets; i++) {
        INIT_HLIST_HEAD(&s->buckets[i].head);
        mutex_init(&s->buckets[i].lock);
    }

    INIT_LIST_HEAD(&s->lru_list);
    spin_lock_init(&s->lru_lock);
    return 0;
}

void kv_store_destroy(struct kv_store *s)
{
    size_t i;
    struct kv_item *item;
    struct hlist_node *tmp;

    for (i = 0; i < s->bucket_count; i++) {
        mutex_lock(&s->buckets[i].lock);
        hlist_for_each_entry_safe(item, tmp, &s->buckets[i].head, hnode) {
            hlist_del(&item->hnode);
            kfree(item);
        }
        mutex_unlock(&s->buckets[i].lock);
    }
    kfree(s->buckets);
}

int kv_put(struct kv_store *s, struct kv_pair *p)
{
    u32 h = hash_key(p->key) % s->bucket_count;
    struct kv_bucket *b = &s->buckets[h];
    struct kv_item *item;

    mutex_lock(&b->lock);
    hlist_for_each_entry(item, &b->head, hnode) {
        if (!strcmp(item->key, p->key)) {
            memcpy(item->value, p->value, p->value_len);
            item->value_len = p->value_len;
            lru_touch(s, item);
            mutex_unlock(&b->lock);
            return 0;
        }
    }

    if (s->use_lru && atomic_read(&s->cur_items) >= s->max_items) {
        struct kv_item *victim = lru_evict(s);
        if (victim) {
            hlist_del(&victim->hnode);
            kfree(victim);
            atomic_dec(&s->cur_items);;
        }
    }

    item = kmalloc(sizeof(*item), GFP_KERNEL);
    if (!item) {
        mutex_unlock(&b->lock);
        return -ENOMEM;
    }

    strscpy(item->key, p->key, KV_MAX_KEY);
    memcpy(item->value, p->value, p->value_len);  
    item->value_len = p->value_len;

    INIT_LIST_HEAD(&item->lru_node);
    hlist_add_head(&item->hnode, &b->head);

    if (s->use_lru) {
        spin_lock(&s->lru_lock);
        list_add(&item->lru_node, &s->lru_list);
        spin_unlock(&s->lru_lock);
    }

    atomic_inc(&s->cur_items);;
    mutex_unlock(&b->lock);
    return 0;
}

int kv_get(struct kv_store *s, struct kv_pair *p)
{
    u32 h = hash_key(p->key) % s->bucket_count;
    struct kv_bucket *b = &s->buckets[h];
    struct kv_item *item;

    mutex_lock(&b->lock);
    hlist_for_each_entry(item, &b->head, hnode) {
        if (!strcmp(item->key, p->key)) {
            memcpy(p->value, item->value, item->value_len);
            p->value_len = item->value_len;
            lru_touch(s, item);
            mutex_unlock(&b->lock);
            return 0;
        }
    }
    mutex_unlock(&b->lock);
    return -ENOENT;
}

int kv_del(struct kv_store *s, struct kv_key *k)
{
    u32 h = hash_key(k->key) % s->bucket_count;
    struct kv_bucket *b = &s->buckets[h];
    struct kv_item *item;

    mutex_lock(&b->lock);

    hlist_for_each_entry(item, &b->head, hnode) {
        if (!strcmp(item->key, k->key)) {

            hlist_del(&item->hnode);

            if (s->use_lru) {
                spin_lock(&s->lru_lock);
                list_del(&item->lru_node);
                spin_unlock(&s->lru_lock);
            }

            kfree(item);
            atomic_dec(&s->cur_items);

            mutex_unlock(&b->lock);
            return 0;
        }
    }

    mutex_unlock(&b->lock);
    return -ENOENT;
}

int kv_stat(struct kv_store *store, struct kv_usage_stat *stat)
{
    if (stat == NULL || store == NULL)
        return -EINVAL;
    
    stat->bucket_count = store->bucket_count;
    stat->max_items = store->bucket_count;
    stat->use_lru = store->use_lru;
    stat->cur_items = atomic_read(&store->cur_items);

    return 0;
}
