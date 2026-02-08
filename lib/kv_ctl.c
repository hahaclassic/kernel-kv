#include <stdio.h>
#include <string.h>
#include "kv_lib.h"

static int cli_put(int fd, const char *key, const char *value) {
    struct kv_pair pair = {0};
    pair.key.len = snprintf(pair.key.data, KV_MAX_KEY, "%s", key);
    pair.value.len = snprintf(pair.value.data, KV_MAX_VAL, "%s", value);

    int ret = kv_put(fd, &pair);
    if (ret) {
        fprintf(stderr, "kv_put error: %s\n", kv_err_msg(ret));
    }
    return ret;
}

static int cli_get(int fd, const char *key) {
    struct kv_pair pair = {0};
    pair.key.len = snprintf(pair.key.data, KV_MAX_KEY, "%s", key);

    int ret = kv_get(fd, &pair);
    if (ret) {
        fprintf(stderr, "kv_get error: %s\n", kv_err_msg(ret));
    } else {
        printf("%.*s\n", pair.value.len, pair.value.data);
    }
    return ret;
}

static int cli_del(int fd, const char *key) {
    struct kv_key k = {0};
    k.len = snprintf(k.data, KV_MAX_KEY, "%s", key);

    int ret = kv_del(fd, &k);
    if (ret) {
        fprintf(stderr, "kv_del error: %s\n", kv_err_msg(ret));
    }
    return ret;
}

static int cli_stat(int fd) {
    struct kv_usage_stat stat;
    int ret = kv_stat(fd, &stat);
    if (ret) {
        fprintf(stderr, "kv_stat error: %s\n", kv_err_msg(ret));
    } else {
        printf("buckets: %lu\nmax_items: %lu\ncurr_items: %lu\nlru: %s\n",
               stat.bucket_count, stat.max_items, stat.cur_items,
               stat.use_lru ? "yes" : "no");
    }
    return ret;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "  kvctl put <key> <value>\n");
        fprintf(stderr, "  kvctl get <key>\n");
        fprintf(stderr, "  kvctl del <key>\n");
        fprintf(stderr, "  kvctl stat\n");
        return 1;
    }

    int fd = kv_open();
    if (fd < 0) {
        perror("kv_open");
        return 1;
    }

    int ret = 0;
    if (!strcmp(argv[1], "put")) {
        if (argc < 4) {
            fprintf(stderr, "put requires key and value\n");
            ret = 1;
        } else {
            ret = cli_put(fd, argv[2], argv[3]);
        }
    } else if (!strcmp(argv[1], "get")) {
        if (argc < 3) {
            fprintf(stderr, "get requires key\n");
            ret = 1;
        } else {
            ret = cli_get(fd, argv[2]);
        }
    } else if (!strcmp(argv[1], "del")) {
        if (argc < 3) {
            fprintf(stderr, "del requires key\n");
            ret = 1;
        } else {
            ret = cli_del(fd, argv[2]);
        }
    } else if (!strcmp(argv[1], "stat")) {
        ret = cli_stat(fd);
    } else {
        fprintf(stderr, "unknown command: %s\n", argv[1]);
        ret = 1;
    }

    kv_close(fd);
    return ret;
}