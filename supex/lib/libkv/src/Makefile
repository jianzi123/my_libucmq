# Makefile for src



OPTIMIZATION?=-O2
DEPENDENCY_TARGETS=hiredis linenoise lua

# Default settings
STD=-std=c99 -pedantic
WARN=-Wall -W
OPT=$(OPTIMIZATION)

MALLOC=jemalloc
LIBJEMALLOC_DIR=../jemalloc
LIBJEMALLOC_CFLAGS=-std=gnu99 -Wall -pipe -g3 -O3 -funroll-loops $(CFLAGS)
LIBJEMALLOC_LDFLAGS=$(LDFLAGS)



FINAL_CFLAGS=$(STD) $(WARN) $(OPT) $(DEBUG) $(CFLAGS) $(LIBKV_CFLAGS)
FINAL_LDFLAGS=$(LDFLAGS) $(LIBKV_LDFLAGS) $(DEBUG)
DEBUG=-g -ggdb

FINAL_CFLAGS+= -DUSE_JEMALLOC -I../jemalloc/include -I/usr/local/include/luajit-2.0

LIBKV_API=libkv.h libkv.a

LIBKV_CC=$(CC) $(FINAL_CFLAGS)
LIBKV_LD=$(CC) $(FINAL_LDFLAGS)
AR=ar

LIBKV_NAME=libkv.a
LIBKV_OBJ=adlist.o dict.o sds.o zmalloc.o lzf_c.o lzf_d.o pqsort.o zipmap.o sha1.o ziplist.o util.o object.o db.o t_string.o t_list.o t_set.o t_hash.o config.o debug.o sort.o intset.o crc16.o endianconv.o rio.o rand.o crc64.o kv.o kv_version.o kv_timer.o

LIBJEMALLOC_NAME=libjemalloc.a
LIBJEMALLOC_OBJ=*.o
LIBJEMALLOC_OBJ_DIR=./.abstract


$(LIBKV_NAME): $(LIBJEMALLOC_NAME) $(LIBKV_OBJ)
	-rm $(LIBJEMALLOC_OBJ_DIR) -fr
	mkdir $(LIBJEMALLOC_OBJ_DIR)
	cp $(LIBJEMALLOC_DIR)/lib/$(LIBJEMALLOC_NAME) $(LIBJEMALLOC_OBJ_DIR)
	cd $(LIBJEMALLOC_OBJ_DIR) && $(AR) x $(LIBJEMALLOC_NAME)
	ar rcs ./$(LIBKV_NAME) $(LIBKV_OBJ) $(LIBJEMALLOC_OBJ_DIR)/$(LIBJEMALLOC_OBJ)

$(LIBJEMALLOC_NAME):
	cd $(LIBJEMALLOC_DIR) && ./configure --with-jemalloc-prefix=je_ --enable-cc-silence CFLAGS="$(LIBJEMALLOC_CFLAGS)" LDFLAGS="$(LIBJEMALLOC_LDFLAGS)"
	cd $(LIBJEMALLOC_DIR) && $(MAKE) CFLAGS="$(LIBJEMALLOC_CFLAGS)" LDFLAGS="$(LIBJEMALLOC_LDFLAGS)" lib/libjemalloc.a

%.o: %.c
	$(LIBKV_CC) -c $<

release:clean $(LIBKV_NAME)
	rm -fr ../release/*
	-mkdir ../release
	cp $(LIBKV_API) ../release/





clean:
	rm -fr *.o *.*~ ./$(LIBKV_NAME) .abstract
#	make -C $(LIBJEMALLOC_DIR) clean
	rm -fr ../release
