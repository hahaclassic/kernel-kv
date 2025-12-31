#pragma once

#include <linux/fs.h>
#include "kv_ioctl.h"

/*
 * File operations for /dev/kvstore
 */

long kv_ioctl(struct file *file,
              unsigned int cmd,
              unsigned long arg);

extern const struct file_operations kv_fops;