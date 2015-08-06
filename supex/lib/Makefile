LIB_DIR=$(shell pwd)
OBJ = libev.a \
      libcoro.a \
      libjson-c.a \
      libcjson.a \
      libtopo.a \
      libsnappy.a \
      libleveldb.a \
      libucmq.a \
      liblrucache.a \
      libkv.a \
      libhiredis.a \
      libtopoi.a


all:$(OBJ)
	-@echo "Done LIB!"

libev.a:
	cd ./libev && sh ./autogen.sh && ./configure
	$(MAKE) -C $(LIB_DIR)/libev
	mv $(LIB_DIR)/libev/.libs/$@ $(LIB_DIR)

libcoro.a:
	$(MAKE) -C $(LIB_DIR)/libcoro
	mv $(LIB_DIR)/libcoro/$@ $(LIB_DIR)

libjson-c.a:
	cd ./json-c && ./autogen.sh && ./configure
	$(MAKE) -C $(LIB_DIR)/json-c
	mv $(LIB_DIR)/json-c/.libs/$@ $(LIB_DIR)

libcjson.a:
	$(MAKE) -C $(LIB_DIR)/cJSON
	mv $(LIB_DIR)/cJSON/$@ $(LIB_DIR)

libtopo.a:
	$(MAKE) -C $(LIB_DIR)/libtopo
	mv $(LIB_DIR)/libtopo/$@ $(LIB_DIR)
libtopoi.a:
	$(MAKE) -C $(LIB_DIR)/libtopoi
	mv $(LIB_DIR)/libtopoi/$@ $(LIB_DIR)

libsnappy.a:
	-if [ -d ./snappy-1.1.1 ];then  echo "snappy-1.1.1 exist!"; else  tar -xzvf ../open/install/snappy-1.1.1.tar.gz; fi
	cd ./snappy-1.1.1 && ./configure
	$(MAKE) -C $(LIB_DIR)/snappy-1.1.1
	mv $(LIB_DIR)/snappy-1.1.1/.libs/$@ $(LIB_DIR)

libleveldb.a:
	$(MAKE) -C $(LIB_DIR)/leveldb-1.15.0
	mv $(LIB_DIR)/leveldb-1.15.0/$@ $(LIB_DIR)

libucmq.a:
	$(MAKE) -C $(LIB_DIR)/libucmq
	mv $(LIB_DIR)/libucmq/$@ $(LIB_DIR)

liblrucache.a:
	$(MAKE) -C $(LIB_DIR)/liblrucache
	mv $(LIB_DIR)/liblrucache/$@ $(LIB_DIR)
libkv.a:
	$(MAKE) -C $(LIB_DIR)/libkv release
	mv $(LIB_DIR)/libkv/release/$@ $(LIB_DIR)

libhiredis.a:
	$(MAKE) -C $(LIB_DIR)/hiredis
	mv $(LIB_DIR)/hiredis/$@ $(LIB_DIR)

test:
	gcc -g -o example -I./libev -I./hiredis -L./  hiredis/examples/example-libev.c  -lev -lhiredis -lm

clean:
	-if [ -f libev/Makefile ];then cd ./libev && make distclean; else cd ./libev && sh ./autogen.sh && ./configure && make distclean; fi
	-if [ -f json-c/Makefile ];then cd ./json-c && make distclean; fi
	-if [ -d ./snappy-1.1.1 ];then rm -rf ./snappy-1.1.1; fi
	$(MAKE) -C $(LIB_DIR)/leveldb-1.15.0 clean
	$(MAKE) -C $(LIB_DIR)/cJSON clean
	$(MAKE) -C $(LIB_DIR)/libucmq clean
	$(MAKE) -C $(LIB_DIR)/libcoro clean
	$(MAKE) -C $(LIB_DIR)/libtopo clean
	$(MAKE) -C $(LIB_DIR)/libtopoi clean
	$(MAKE) -C $(LIB_DIR)/liblrucache clean
	$(MAKE) -C $(LIB_DIR)/hiredis clean
	$(MAKE) -C $(LIB_DIR)/libkv clean
	rm -rf $(OBJ)

