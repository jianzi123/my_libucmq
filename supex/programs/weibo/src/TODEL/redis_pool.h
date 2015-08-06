#pragma once
#include <ev.h>
#include <adapters/libev.h>
#include <hiredis.h>
#include <async.h>

#include "cnt_pool.h"


void redis_pool_init(char *host, int port);

void *redis_pool_gain(struct ev_loop *loop, struct cnt_pool **pool, char *host, int port);

void default_callback(redisAsyncContext *c, void *r, void *privdata);

void callback_data(redisAsyncContext *c, void *r, void *privdata);
