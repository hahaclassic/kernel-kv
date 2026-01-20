#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <inttypes.h>
#include "kv_lib.h"

#define N_CLIENTS 5
#define N_KEYS    10

typedef struct {
    int fd;
    int id;
} client_arg_t;

void *client_thread(void *arg) {
    client_arg_t *c = (client_arg_t *)arg;
    int fd = c->fd;
    int id = c->id;
    char key[64], val[64];
    struct kv_pair p;

    for (int i = 0; i < N_KEYS; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(val, sizeof(val), "client%d_value%d", id, i);

        // PUT
        strncpy(p.key.data, key, sizeof(p.key.data));
        p.key.len = strlen(p.key.data);
        strncpy(p.value.data, val, sizeof(p.value.data));
        p.value.len = strlen(p.value.data);

        if (kv_put(fd, &p) != 0) {
            fprintf(stderr, "[Client %d] PUT error for %s\n", id, key);
        }

        // GET
        strncpy(p.key.data, key, sizeof(p.key.data));
        p.key.len = strlen(p.key.data);
        if (kv_get(fd, &p) == 0) {
            printf("[Client %d] GET: %.*s -> %.*s\n", 
                id, (int)p.key.len, p.key.data, (int)p.value.len, p.value.data);
        } else {
            printf("[Client %d] GET: %s not found\n", id, key);
        }

        // Occasionally delete
        if (i % 3 == 0) {
            if (kv_del(fd, &p.key) == 0) {
                printf("[Client %d] DEL: %s\n", id, key);
            }
        }

        usleep(1000 * (rand() % 10)); // небольшая задержка
    }

    return NULL;
}

void print_stat(int fd)
{
    struct kv_usage_stat stat = {0};
    if (kv_stat(fd, &stat) < 0) {
        perror("kv_stat");
    }
    
    printf("STAT:\n\tbucket count: %" PRIu64 "\n\tmax_items: %" PRIu64 "\n\tcurr_items: %" PRIu64 "\n\tlru: %s\n", 
        stat.bucket_count, stat.max_items, stat.cur_items, stat.use_lru ? "on" : "off");
}

int main(void) {
    int fd = kv_open();
    if (fd < 0) {
        perror("kv_open");
        return 1;
    }

    pthread_t threads[N_CLIENTS];
    client_arg_t args[N_CLIENTS];

    for (int i = 0; i < N_CLIENTS; i++) {
        args[i].fd = fd;
        args[i].id = i + 1;
        pthread_create(&threads[i], NULL, client_thread, &args[i]);
    }

    for (int i = 0; i < N_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final storage stats:\n");
    print_stat(fd);

    kv_close(fd);
    return 0;
}

