# Makefile for unit test
#
# make or make target=ut_*
#



UT_LIB=-lcunit
KV_LIB=../release/libkv.a -lpthread
KV_INCLUDE=-I../release/

CFLAGS+=$(KV_INCLUDE) -g
CC=gcc

AUTO_FLAG=
UT_NAME=ut_auto

UT_OBJS=ut_public.o \
		ut_data_generator.o \
		ut_set.o \
		ut_get.o \
		ut_del.o \
		ut_lpush.o \
		ut_lrange.o \
		ut_sadd.o \
		ut_smembers.o \
		ut_zadd.o \
		ut_zrange.o \
		ut_hset.o \
		ut_hget.o \
		ut_exists.o \
		ut_scard.o \
		ut_select.o \
		ut_version.o \
		ut_auto.o \
		ut_sismember.o \
		ut_type.o \
		ut_init.o \
		ut_dbsize.o \
		ut_flushdb.o \
		ut_incr.o \
		ut_incrby.o \
		ut_decr.o \
		ut_decrby.o \
		ut_used_mem.o \
		ut_expire.o \
		ut_expireat.o \
		ut_pexpire.o \
		ut_pexpireat.o


ifdef target
ifeq ($(target),ut_auto)
CFLAGS+=-DENABLE_AUTO
else
UT_OBJS=$(target).o ut_public.o ut_data_generator.o
endif
else # default
CFLAGS+=-DENABLE_AUTO 
target=ut_auto
endif



all:$(UT_OBJS)
	$(CC) -o $(target) $(UT_OBJS) $(KV_LIB) $(UT_LIB) $(CFLAGS) -Wall

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(KV_INCLUDE) $(AUTO_FLAG)





clean:
	rm -f *.o *.*~

.PHONY: clean

