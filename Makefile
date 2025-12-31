# Имя модуля
obj-m := kv.o

# Состав модуля
kv-objs := \
	kv_mod.o \
	kv_dev.o \
	kv_store.o \
	kv_lru.o

# Путь к заголовкам текущего ядра
KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

# Сборка
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Очистка
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

# Удобные таргеты (необязательно, но полезно)
load:
	sudo insmod kv.ko buckets=128 max_items=100 use_lru=1

unload:
	sudo rmmod kv

reload: unload load

log:
	dmesg | tail -n 50
