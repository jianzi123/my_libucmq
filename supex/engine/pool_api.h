#pragma once

#include <ev.h>

#include "cnt_pool.h"

void pool_api_init(char *host, int port);

void *pool_api_gain(struct ev_loop *loop, struct cnt_pool **pool, char *host, int port);
