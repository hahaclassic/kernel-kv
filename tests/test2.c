#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "kv_lib.h"
#include <inttypes.h>

void print_stat(int fd)
{
    struct kv_usage_stat stat = {0};
    if (kv_stat(fd, &stat) < 0) {
        perror("kv_stat");
    }
    
    printf("STAT:\n\tbucket count: %" PRIu64 "\n\tmax_items: %" PRIu64 "\n\tcurr_items: %" PRIu64 "\n\tlru: %s\n", 
        stat.bucket_count, stat.max_items, stat.cur_items, stat.use_lru ? "on" : "off");
}

int main(void)
{
    int err;
    int fd = kv_open();
    if (fd < 0) {
        perror("kv_open");
        return 1;
    }

    // Fill storage
    print_stat(fd);
    for (int i = 1; i <= 11; i++) {
        struct kv_pair p;
        
        snprintf(p.key.data, sizeof(p.key), "key%d", i);
        p.key.len = strlen(p.key.data)+1;
        snprintf(p.value.data, sizeof(p.value), "value%d", i);
        p.value.len = strlen(p.value.data)+1;

        err = kv_put(fd, &p);
        if (err < 0) {
            perror("kv_put");
            printf("%s\n", kv_err_msg(-errno));
        } else {
            printf("PUT: %s -> %s\n", p.key.data, p.value.data);
        }
    }

    print_stat(fd);

    // Check key1
    struct kv_pair p;
    strcpy(p.key.data, "key1\0");
    p.key.len = 5;

    err = kv_get(fd, &p);
    if (err == 0) {
        printf("ERROR: key1 still present: %s\n", p.value.data);
    } else {
        printf("err: %s\n", kv_err_msg(-errno));
        printf("OK: key1 was evicted\n");
    }

    // 3. Get key2
    strcpy(p.key.data, "key2\0");
    err = kv_get(fd, &p);
    if (err == 0) {
        printf("GET: key2 -> %s\n", p.value.data);
    } else {
        printf("ERROR: key2 not found: %s\n", kv_err_msg(-errno));
    }

    // 4. Delete key2
    err = kv_del(fd, &p.key);
    if (err == 0) {
        printf("DEL: key2\n");
    } else {
        printf("err: %s\n", kv_err_msg(-errno));
        perror("kv_del key2");
    }

    // 5. Check key2
    err = kv_get(fd, &p);
    if (err) {
        err = -errno;
        if (err == -ENOENT)
            printf("OK: key2 correctly deleted\n");
        else
            printf("err: %s\n", kv_err_msg(err));
    } else {
        printf("ERROR: key2 still exists\n");
    }

    print_stat(fd);

    kv_close(fd);
    return 0;
}
