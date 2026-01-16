#include <linux/fs.h>
#include <linux/uaccess.h>
#include "kv_dev.h"
#include "kv_store.h"

extern struct kv_store store;

long kv_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case KV_PUT:
        struct kv_pair p;
        if (copy_from_user(&p, (void __user *)arg, sizeof(p)))
            return -EFAULT;
        pr_info("kv_ioctl: KV_PUT key=%s value=%s\n", p.key, p.value);

        return kv_put(&store, &p);

    case KV_GET:
        struct kv_pair p;
        if (copy_from_user(&p, (void __user *)arg, sizeof(p)))
            return -EFAULT;
            
        pr_info("kv_ioctl: KV_GET key=%s\n", p.key);
        int err = kv_get(&store, &p);
        if (err != 0)
            return -ENOENT;
        
        return copy_to_user((void __user *)arg, &p, sizeof(p));

    case KV_DEL:
        struct kv_key k;
        if (copy_from_user(&k, (void __user *)arg, sizeof(p)))
            return -EFAULT;
        pr_info("kv_ioctl: KV_DEL key=%s\n", p.key);
        
        return kv_del(&store, &p);

    case KV_STAT:
        pr_info("kv_ioctl: KV_STAT");
        
        struct kv_usage_stat stat;
        int err = kv_stat(&store, &stat);
        if (err != NULL)
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
