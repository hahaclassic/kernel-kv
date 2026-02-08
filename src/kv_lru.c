#include "kv_lru.h"

void lru_init(struct kv_lru *lru) 
{
    INIT_LIST_HEAD(&lru->head);
}

void lru_touch(struct kv_lru *lru, struct kv_item *item)
{
    if (list_empty(&item->lru_node))
        list_add(&item->lru_node, &lru->head); 
    else
        list_move(&item->lru_node, &lru->head);
}

void lru_remove(struct kv_lru *lru, struct kv_item *item)
{
    if (!list_empty(&item->lru_node))
        list_del_init(&item->lru_node);
}

struct kv_item *lru_evict(struct kv_lru *lru)
{
    struct kv_item *victim = NULL;

    if (!list_empty(&lru->head)) {
        victim = list_last_entry(&lru->head, struct kv_item, lru_node);
        list_del_init(&victim->lru_node);
    }
    
    return victim;
}
