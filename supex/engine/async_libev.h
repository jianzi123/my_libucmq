#pragma once

#include <ev.h>



struct async_events {
	struct async_ctx *context;
	struct ev_loop *loop;
	int reading, writing;
	ev_io rev, wev;
};

int async_init(struct async_ctx *ac);

int async_bind(struct ev_loop *loop, struct async_ctx *ac);
