# Kernel KV Store

A simple in-memory key-value store implemented as 
a Linux kernel module with optional LRU eviction.

## Operations (API)

1. `put` — add or update a key-value pair
2. `get` — retrieve a value by key
3. `del` — delete a key
4. `stat` — view bucket count, max/current items, and LRU usage

| Operation | Userspace Library | CLI |
|-----------|-----------------|-----|
| put       | `int kv_put(int fd, const struct kv_pair *pair)` | `kvctl put <key> <value>` |
| get       | `int kv_get(int fd, struct kv_pair *pair)`  | `kvctl get <key>` |
| del       | `int kv_del(int fd, const struct kv_key *key)`  | `kvctl del <key>` |
| stat      | `int kv_stat(int fd, struct kv_usage_stat *stat_out)` | `kvctl stat` |
| open      | `int kv_open()` | - |
| close      | `int kv_close(int fd)` | - |

## Key and Value Limits

- Maximum key length: 64 bytes (`KV_MAX_KEY`)  
- Maximum value length: 256 bytes (`KV_MAX_VAL`)  
- Using the **userspace library**, keys and values can be any byte sequences  
- Using the **CLI (`kvctl`)**, keys and values are treated as strings; non-printable bytes may not be supported  

<!-- ## Performance Notes -->

> [!NOTE]
> Since the store is implemented using the Linux kernel's `hashtable.h`, when using custom `buckets` and `max_items`, **load factor (`max_items / buckets`) should generally not exceed 2** to reduce hash > collisions and maintain performance.

## Build and Install Module

Build and install the kernel module:

```bash
make

sudo insmod kv.ko buckets=128 max_items=256 use_lru=1
```

Parameters `buckets`, `max_items`, `use_lru` are optional. Default values: 
- buckets = 128
- max_items = 256
- use_lru = 0 (1 - yes, 0 - no)

Remove the module:

```bash
sudo rmmod kv
# OR
make unload
```

## Userspace Library and CLI

Build dynamic library:
```bash
make build-lib-so
```

Build CLI:
```bash
make build-kvctl
```

To run CLI from any directory, copy it to a directory in your PATH:

```bash
sudo cp kvctl /usr/local/bin/
sudo chmod +x /usr/local/bin/kvctl
```

To run CLI without `sudo`, make `/dev/kvstore` readable/writable:
```bash
sudo chmod 666 /dev/kvstore
```

Example usage:

```bash
# Using CLI
kvctl put hello world
kvctl get hello
# WORLD
kvctl stat
# buckets: 128
# max_items: 100
# curr_items: 1
# lru: yes
kvctl del hello
kvctl get hello
# kv_get error: Key not found
kvctl stat
# buckets: 128
# max_items: 100
# curr_items: 0
# lru: yes
```