obj-m := kv.o

kv-objs := \
	src/kv_store.o \
	src/kv_lru.o \
	src/kv_mod.o

ccflags-y += -I$(src)/inc

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

build-module:
	$(MAKE) -C $(KDIR) M=$(PWD) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod kv.ko buckets=128 max_items=256 use_lru=1

unload:
	sudo rmmod kv

reload: unload load

# Userspace lib
build-lib-so:
	gcc -fPIC -I ./lib -c ./lib/kv_lib.c -o kv_lib.o
	gcc -shared -o libkernelkv.so kv_lib.o

build-tests:
	gcc ./tests/test1.c ./lib/kv_lib.c -o test1 -I ./lib
	gcc ./tests/test2.c ./lib/kv_lib.c -o test2 -I ./lib 
	gcc ./tests/test3.c ./lib/kv_lib.c -o test3 -I ./lib  

clean-tests:
	rm -f ./test1 ./test2 ./test3

# cli
build-kvctl:
	gcc ./lib/kv_ctl.c ./lib/kv_lib.c -I ./lib -o kvctl

dev-chmod:
	sudo chmod 666 /dev/kvstore
