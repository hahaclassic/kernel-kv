#include <linux/module.h>
#include <linux/miscdevice.h>
#include "kv_store.h"
#include "kv_dev.h"

static int buckets = 64;
static int max_items = 1024;
static bool use_lru = true;

module_param(buckets, int, 0444);
module_param(max_items, int, 0444);
module_param(use_lru, bool, 0444);

struct kv_store store;

static struct miscdevice kv_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "kvstore",
    .fops = &kv_fops,
};

static int __init kv_init(void)
{
    kv_store_init(&store, buckets, max_items, use_lru);
    return misc_register(&kv_dev);
}

static void __exit kv_exit(void)
{
    misc_deregister(&kv_dev);
    kv_store_destroy(&store);
}

module_init(kv_init);
module_exit(kv_exit);
MODULE_LICENSE("GPL");
