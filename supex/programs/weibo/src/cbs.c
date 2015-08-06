#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "utils.h"
#include "redis_parser.h"
#include "async_api.h"
#include "async_libev.h"
#include "cbs.h"




void default_callback(struct async_ctx *ac, void *reply, void *data)
{
        struct async_ctx *c = (struct async_ctx*)ac;
        struct redis_reply *r = (struct redis_reply*)reply;

        if (c->err) {
                cnt_free((struct cnt_pool *)data, (uintptr_t **)&ac );
        } else {
                struct async_events *e = (struct async_events*)c->ev.data;
                struct ev_loop *loop = (struct ev_loop*)e->loop;
                if (e->reading) {
                        e->reading = 0;
                        ev_io_stop(loop, &e->rev);
                }
                if (e->writing) {
                        e->writing = 0;
                        ev_io_stop(loop, &e->wev);
                }

                cnt_push ( (struct cnt_pool *)data, (uintptr_t **)&ac );
        }
}
