#include <stdlib.h>
#include <sys/types.h>

#include "async_api.h"
#include "async_libev.h"



static void libev_recv_event(struct ev_loop *loop, ev_io *watcher, int revents) {
#if EV_MULTIPLICITY
	((void)loop);
#endif
	((void)revents);

	struct async_events *e = (struct async_events*)watcher->data;
	_async_handle_recv(e->context);
}

static void libev_send_event(struct ev_loop *loop, ev_io *watcher, int revents) {
#if EV_MULTIPLICITY
	((void)loop);
#endif
	((void)revents);

	struct async_events *e = (struct async_events*)watcher->data;
	_async_handle_send(e->context);
}

static void libev_add_recv(void *privdata) {
	struct async_events *e = (struct async_events*)privdata;
	struct ev_loop *loop = e->loop;
	((void)loop);
	if (!e->reading) {
		e->reading = 1;
		ev_io_start(EV_A_ &e->rev);
	}
}

static void libev_del_recv(void *privdata) {
	struct async_events *e = (struct async_events*)privdata;
	struct ev_loop *loop = e->loop;
	((void)loop);
	if (e->reading) {
		e->reading = 0;
		ev_io_stop(EV_A_ &e->rev);
	}
}

static void libev_add_send(void *privdata) {
	struct async_events *e = (struct async_events*)privdata;
	struct ev_loop *loop = e->loop;
	((void)loop);
	if (!e->writing) {
		e->writing = 1;
		ev_io_start(EV_A_ &e->wev);
	}
}

static void libev_del_send(void *privdata) {
	struct async_events *e = (struct async_events*)privdata;
	struct ev_loop *loop = e->loop;
	((void)loop);
	if (e->writing) {
		e->writing = 0;
		ev_io_stop(EV_A_ &e->wev);
	}
}

static void libev_cleanup(void *privdata) {
	struct async_events *e = (struct async_events*)privdata;
	libev_del_recv(privdata);
	libev_del_send(privdata);
	free(e);
}




int async_init(struct async_ctx *ac)
{
	/* Nothing should be attached when something is already attached */
	if (ac->ev.data != NULL)
		return ASYNC_ERR;

	/* Create container for context and r/w events */
	struct async_events *e = (struct async_events*)malloc(sizeof(*e));
	if (e == NULL)
		return ASYNC_ERR;
	e->context = ac;
	e->rev.data = e;
	e->wev.data = e;

	/* Register functions to start/stop listening for events */
	ac->ev.add_recv = libev_add_recv;
	ac->ev.del_recv = libev_del_recv;
	ac->ev.add_send = libev_add_send;
	ac->ev.del_send = libev_del_send;
	ac->ev.cleanup = libev_cleanup;
	ac->ev.data = e;

	/* Initialize read/write events */
	ev_io_init(&e->rev, libev_recv_event, ac->sfd, EV_READ);/*TODO:check can reuse*/
	ev_io_init(&e->wev, libev_send_event, ac->sfd, EV_WRITE);
	return ASYNC_OK;
}

int async_bind(struct ev_loop *loop, struct async_ctx *ac)
{
	struct async_events *e = (struct async_events *)ac->ev.data;
	if (e == NULL)
		return ASYNC_ERR;
#if EV_MULTIPLICITY
	e->loop = loop;
#else
	e->loop = NULL;
#endif
	e->reading = e->writing = 0;
	return ASYNC_OK;
}
