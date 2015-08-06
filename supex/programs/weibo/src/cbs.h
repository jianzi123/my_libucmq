#pragma once
#include <ev.h>

#include "cnt_pool.h"

struct async_ctx;



void default_callback(struct async_ctx *ac, void *reply, void *data);
