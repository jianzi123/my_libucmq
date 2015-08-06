#pragma once

#include <ev.h>

#include "smart_task.h"


void smart_dispatch_task(int type, int sfd);

void smart_all_task_hit( struct smart_task_node *task, bool synch, int mark );

void smart_one_task_hit( struct smart_task_node *task, bool synch, int mark, unsigned int step );

void smart_accept_cb	(struct ev_loop *loop, ev_io *w, int revents);
void smart_prepare_cb	(struct ev_loop *loop, ev_prepare *w, int revents);
void smart_async_cb	(struct ev_loop *loop, ev_async *w, int revents);
void smart_check_cb	(struct ev_loop *loop, ev_check *w, int revents);
/*
 * 名  称:smart_monitor_cb
 * 功  能:周期循环事件的回调函数
 * 参  数:loop 事件循环，　w　周期循环事件，　
 * 返回值:
 * 修  改:添加新函数,程少远 2015/05/12
 */
void smart_monitor_cb(struct ev_loop* loop, ev_periodic* w, int revents);
void smart_update_cb	(struct ev_loop *loop, ev_timer *w, int revents);
void smart_signal_cb	(struct ev_loop *loop, ev_signal *w, int revents);
void smart_fetch_cb	(struct ev_loop *loop, ev_io *w, int revents);
