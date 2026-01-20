#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "kv_lib.h"

#define NUM_KEYS 100
#define NUM_READERS 4
#define NUM_ITER 1000

void *writer_thread(void *arg) {
    int fd = kv_open();
    struct kv_pair p;
    for (int i = 0; i < NUM_ITER; i++) {
        for (int k = 0; k < NUM_KEYS; k++) {
            snprintf(p.key.data, KV_MAX_KEY, "key%d", k);
            p.key.len = strlen(p.key.data);
            snprintf(p.value.data, KV_MAX_VAL, "val%d_%d", k, i);
            p.value.len = strlen(p.value.data);
            int err = kv_put(fd, &p);
            if (err != 0) {
                perror("kv_put");
                printf("PUT ERR: %s -> %s: %s\n",
                    p.key.data, p.value.data, kv_err_msg(err));
            } else {
                printf("PUT OK: %s -> %s: %s\n",
                    p.key.data, p.value.data, kv_err_msg(err));
            }
        }
    }
    kv_close(fd);
    return NULL;
}

void *reader_thread(void *arg) {
    int fd = kv_open();
    struct kv_pair p;
    for (int i = 0; i < NUM_ITER; i++) {
        for (int k = 0; k < NUM_KEYS; k++) {
            snprintf(p.key.data, KV_MAX_KEY, "key%d", k);
            p.key.len = strlen(p.key.data);
            int err = kv_get(fd, &p);
            if (err != 0) {
                perror("kv_get");
                printf("GET ERR: %s -> %s: %s\n",
                    p.key.data, p.value.data, kv_err_msg(err));
            } else {
                printf("GET OK: %s -> %s: %s\n",
                    p.key.data, p.value.data, kv_err_msg(err));
            }
        }
    }
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
