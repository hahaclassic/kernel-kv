#ifndef KV_IOCTL_H
#define KV_IOCTL_H

#include <linux/ioctl.h>
#include "kv_types.h"

#define KV_IOC_MAGIC 'k'

#define KV_PUT _IOW(KV_IOC_MAGIC, 1, struct kv_pair)
#define KV_GET _IOWR(KV_IOC_MAGIC, 2, struct kv_pair)
#define KV_DEL _IOW(KV_IOC_MAGIC, 3, struct kv_key)
#define KV_STAT _IOR(KV_IOC_MAGIC, 4, struct kv_usage_stat)

#endif // KV_IOCTL_H
