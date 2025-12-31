#include <linux/fs.h>
#include <linux/uaccess.h>
#include "kv_dev.h"
#include "kv_store.h"

extern struct kv_store store;

long kv_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    struct kv_pair p;

    if (copy_from_user(&p, (void __user *)arg, sizeof(p)))
        return -EFAULT;

    switch (cmd) {
    case KV_PUT:
        pr_info("kv_ioctl: KV_PUT key=%s value=%s\n", p.key, p.value);
        return kv_put(&store, &p);
    case KV_GET:
        pr_info("kv_ioctl: KV_GET key=%s\n", p.key);
        if (kv_get(&store, &p) == 0)
            return copy_to_user((void __user *)arg, &p, sizeof(p));
        return -ENOENT;
    case KV_DEL:
        pr_info("kv_ioctl: KV_DEL key=%s\n", p.key);
        return kv_del(&store, &p);
    default:
        pr_info("kv_ioctl: unknown command %u\n", cmd);
        return -EINVAL;
    }
}

const struct file_operations kv_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = kv_ioctl,
};
