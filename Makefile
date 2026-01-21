obj-m := kv.o

kv-objs := \
	kv_store.o \
	kv_lru.o \
	kv_mod.o \

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod kv.ko buckets=128 max_items=100 use_lru=1

unload:
	sudo rmmod kv

reload: unload load
