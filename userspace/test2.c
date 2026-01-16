#include <stdio.h>
#include <string.h>
#include "kv_lib.h"

void print_stat(int fd)
{
    kv_stat_t stat = {0};
    if (kv_stat(fd, &stat) < 0) {
        perror("kv_stat");
    }
    
    printf("STAT:\n\tbucket count: %llu\n\tmax_items: %llu\n\tcurr_items: %llu\n\tlru: %s", 
        stat.bucket_count, stat.max_items, stat.cur_items, stat.use_lru ? "on" : "off");
}

int main(void)
{
    int fd = kv_open();
    if (fd < 0) {
        perror("kv_open");
        return 1;
    }
    print_stat(fd);

    if (kv_put(fd, "hello", "world") < 0) {
        perror("kv_put");
    }
    print_stat(fd);

    char buf[128] = {0};
    if (kv_get(fd, "hello", buf) == 0) {
        printf("GET: hello -> %s\n", buf);
    } else {
        perror("kv_get");
    }
    print_stat(fd);

    if (kv_del(fd, "hello") == 0) {
        printf("Key 'hello' deleted successfully\n");
    } else {
        perror("kv_del");
    }
    print_stat(fd);

    kv_close(fd);
    return 0;
}
