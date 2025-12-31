#include "kv_store.h"

void lru_touch(struct kv_store *s, struct kv_item *item)
{
    if (!s->use_lru)
        return;

    spin_lock(&s->lru_lock);
    list_move(&item->lru_node, &s->lru_list);
    spin_unlock(&s->lru_lock);
}

struct kv_item *lru_evict(struct kv_store *s)
{
    struct kv_item *victim = NULL;

    spin_lock(&s->lru_lock);
    if (!list_empty(&s->lru_list)) {
        victim = list_last_entry(&s->lru_list, struct kv_item, lru_node);
        list_del(&victim->lru_node);
    }
    spin_unlock(&s->lru_lock);
    return victim;
}
