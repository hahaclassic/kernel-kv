#pragma once
int kv_open(void);
int kv_put(int fd, const char *k, const char *v);
int kv_get(int fd, const char *k, char *out);