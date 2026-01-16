#include <linux/fs.h>
#include <linux/uaccess.h>
#include "kv_dev.h"
#include "kv_store.h"

extern struct kv_store store;

long kv_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    int err;
    struct kv_pair p;
    struct kv_key k;
    struct kv_usage_stat stat;

    switch (cmd) {
    case KV_PUT:
        if (copy_from_user(&p, (void __user *)arg, sizeof(p)))
            return -EFAULT;
        pr_info("kv_ioctl: KV_PUT key=%s value=%s\n", p.key, p.value);

        return kv_put(&store, &p);

    case KV_GET:
        if (copy_from_user(&p, (void __user *)arg, sizeof(p)))
            return -EFAULT;
            
        pr_info("kv_ioctl: KV_GET key=%s\n", p.key);
        err = kv_get(&store, &p);
        if (err != 0)
            return -ENOENT;
        
        return copy_to_user((void __user *)arg, &p, sizeof(p));

    case KV_DEL:
        if (copy_from_user(&k, (void __user *)arg, sizeof(k)))
            return -EFAULT;
        pr_info("kv_ioctl: KV_DEL key=%s\n", k.key);
        
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
