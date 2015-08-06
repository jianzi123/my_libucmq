#pragma once

#include <ev.h>

#include "swift_task.h"



void swift_all_task_hit( struct swift_task_node *task, bool synch, int mark );

void swift_one_task_hit( struct swift_task_node *task, bool synch, int mark );

void swift_accept_cb	(struct ev_loop *loop, ev_io *w, int revents);
void swift_prepare_cb	(struct ev_loop *loop, ev_prepare *w, int revents);
void swift_idle_cb	(struct ev_loop *loop, ev_idle *w, int revents);
void swift_async_cb	(struct ev_loop *loop, ev_async *w, int revents);
void swift_check_cb	(struct ev_loop *loop, ev_check *w, int revents);
void swift_update_cb	(struct ev_loop *loop, ev_timer *w, int revents);
void swift_signal_cb	(struct ev_loop *loop, ev_signal *w, int revents);
void swift_fetch_cb	(struct ev_loop *loop, ev_io *w, int revents);
