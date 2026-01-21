#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include "kv_lib.h"

#define NUM_KEYS 3
#define NUM_READERS 4
#define NUM_ITER 4

void print_stat(int fd)
{
    struct kv_usage_stat stat = {0};
    if (kv_stat(fd, &stat) < 0) {
        perror("kv_stat");
    }
    
    printf("[tid=%ld] STAT:\n\tbucket count: %" PRIu64 "\n\tmax_items: %" PRIu64 "\n\tcurr_items: %" PRIu64 "\n\tlru: %s\n",
        pthread_self(), stat.bucket_count, stat.max_items, stat.cur_items, stat.use_lru ? "on" : "off");
}

void *writer_thread(void *arg) {
    struct timespec ts;
    ts.tv_sec = 0;         
    ts.tv_nsec = 100000000;

    int fd = kv_open();
    if (fd == -1) {
        perror("kv_open");
        return NULL;
    }
    print_stat(fd);

    struct kv_pair p;
    for (int i = 0; i < NUM_ITER; i++) {
        for (int k = 0; k < NUM_KEYS; k++) {
            snprintf(p.key.data, KV_MAX_KEY, "key%d", k);
            p.key.len = strlen(p.key.data)+1;
            snprintf(p.value.data, KV_MAX_VAL, "val%d_%d", k, i);
            p.value.len = strlen(p.value.data)+1;
            int err = kv_put(fd, &p);
            if (err != 0) {
                err = -errno;
                printf("WRITER [tid=%ld] PUT ERR: %s -> %s: %s\n", pthread_self(),
                    p.key.data, p.value.data, kv_err_msg(err));
            } else {
                printf("WRITER [tid=%ld] PUT OK: %s -> %s\n", pthread_self(),
                    p.key.data, p.value.data);
            }
            nanosleep(&ts, NULL);
        }
    }
    print_stat(fd);

    kv_close(fd);
    return NULL;
}

void *reader_thread(void *arg) {
    struct timespec ts;
    ts.tv_sec = 0;         
    ts.tv_nsec = 300000000;

    int fd = kv_open();
    if (fd == -1) {
        perror("kv_open");
        return NULL;
    }
    print_stat(fd);

    struct kv_pair p;
    for (int k = 0; k < NUM_KEYS; k++) {
        snprintf(p.key.data, KV_MAX_KEY, "key%d", k);
        p.key.len = strlen(p.key.data)+1;
        int err = kv_get(fd, &p);
        if (err != 0) {
            err = -errno;
            printf("READER [tid=%ld] GET ERR: %s: %s\n", pthread_self(),
                p.key.data, kv_err_msg(err));
        } else {
            printf("READER [tid=%ld] GET OK: %s -> %s\n", pthread_self(),
                p.key.data, p.value.data);
        }
        ts.tv_nsec += rand() % 100000;
        nanosleep(&ts, NULL);
    }
    print_stat(fd);

    kv_close(fd);
    return NULL;
}

int main() {
    pthread_t writer;
    pthread_t readers[NUM_READERS];

    pthread_create(&writer, NULL, writer_thread, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_create(&readers[i], NULL, reader_thread, NULL);
    }

    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    printf("Test finished\n");
    return 0;
}
