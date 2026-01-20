#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "kv_store.h"
#include "kv_ioctl.h"

static int buckets = BUCKETS_DEFAULT;
static int max_items = MAX_ITEMS_DEFAULT;
static bool use_lru = USE_LRU_DEFAULT;

module_param(buckets, int, 0444);
module_param(max_items, int, 0444);
module_param(use_lru, bool, 0444);

struct kv_store store;

static long kv_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    int err;
    struct kv_pair p;
    struct kv_key k;
    struct kv_usage_stat stat;

    switch (cmd) {
    case KV_PUT:
        if (copy_from_user(&p, (void __user *)arg, sizeof(p)))
            return -EFAULT;
        pr_info("kv_ioctl: KV_PUT key=%.*s value=%.*s\n", 
            (int) p.key.len, p.key.data, (int) p.value.len, p.value.data);

        return kv_put(&store, &p);

    case KV_GET:
        if (copy_from_user(&p, (void __user *)arg, sizeof(p)))
            return -EFAULT;
            
        pr_info("kv_ioctl: KV_GET key=%.*s\n", (int) p.key.len, p.key.data);
        err = kv_get(&store, &p);
        if (err != 0)
            return err;
        
        return copy_to_user((void __user *)arg, &p, sizeof(p));

    case KV_DEL:
        if (copy_from_user(&k, (void __user *)arg, sizeof(k)))
            return -EFAULT;
        pr_info("kv_ioctl: KV_DEL key=%.*s\n", (int) k.len, k.data);
        
        return kv_del(&store, &k);

    case KV_STAT:
        pr_info("kv_ioctl: KV_STAT");
        err = kv_stat(&store, &stat);
        if (err != 0)
            return err;

        return copy_to_user((void __user *)arg, &stat, sizeof(stat));
        
    default:
        pr_info("kv_ioctl: unknown command %u\n", cmd);
        return -EINVAL;
    }
}

const struct file_operations kv_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = kv_ioctl,
};

static struct miscdevice kv_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "kvstore",
    .fops = &kv_fops,
};

static int __init kv_init(void)
{
    pr_info("kv_kernel: init storage...");
    int err = kv_store_init(&store, buckets, max_items, use_lru);
    if (err) {
        pr_err("kv_kernel: init failed");
        return err;
    }
    err = misc_register(&kv_dev);
    if (err) {
        pr_err("kv_kernel: misc_register failed");
        return err;
    }
    pr_info("kv_kernel: init storage - DONE.");
    pr_info("");

    return 0;
}

static void __exit kv_exit(void)
{
    pr_info("kv_storage: destroy storage");
    misc_deregister(&kv_dev);
    kv_store_destroy(&store);
    pr_info("kv_kernel: destroy storage - DONE");
    pr_info("");
}

module_init(kv_init);
module_exit(kv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gavrilyuk Vladislav");
