#include <stdio.h>
#include <string.h>
#include "kv_lib.h"

void print_stat(int fd)
{
    kv_stat_t stat = {0};
    if (kv_stat(fd, &stat) < 0) {
        perror("kv_stat");
    }
    
    printf("STAT:\n\tbucket count: %llu\n\tmax_items: %llu\n\tcurr_items: %llu\n\tlru: %s\n", 
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

    /* 1. Загружаем 11 элементов */
    for (int i = 1; i <= 11; i++) {
        char key[16];
        char value[16];

        snprintf(key, sizeof(key), "key%d", i);
        snprintf(value, sizeof(value), "value%d", i);

        if (kv_put(fd, key, value) < 0) {
            perror("kv_put");
        } else {
            printf("PUT: %s -> %s\n", key, value);
        }
    }

    print_stat(fd);

    /* 2. Проверяем, что key1 вытеснён */
    char buf[128] = {0};
    if (kv_get(fd, "key1", buf) == 0) {
        printf("ERROR: key1 still present: %s\n", buf);
    } else {
        printf("OK: key1 was evicted\n");
    }

    /* 3. key2 должен существовать */
    if (kv_get(fd, "key2", buf) == 0) {
        printf("GET: key2 -> %s\n", buf);
    } else {
        printf("ERROR: key2 not found\n");
    }

    /* 4. Удаляем key2 */
    if (kv_del(fd, "key2") == 0) {
        printf("DEL: key2\n");
    } else {
        perror("kv_del key2");
    }

    /* 5. Проверяем, что key2 действительно удалён */
    if (kv_get(fd, "key2", buf) < 0) {
        printf("OK: key2 correctly deleted\n");
    } else {
        printf("ERROR: key2 still exists\n");
    }

    print_stat(fd);

    kv_close(fd);
    return 0;
}
