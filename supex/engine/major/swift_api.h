#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "list.h"
#include "net_cache.h"
#include "utils.h"
#include "coro.h"
#include "swift_task.h"
#include "swift_cfg.h"
#include "supex.h"



struct swift_settings {
	struct swift_cfg_list *conf;
#ifdef USE_HTTP_PROTOCOL
	struct api_list apis[ MAX_API_COUNTS + 1 ];/*all api counts can't big then MAX_API_COUNTS*/
#endif
#ifdef USE_REDIS_PROTOCOL
	struct cmd_list cmds[ MAX_CMD_COUNTS + 1 ];/*all cmd counts can't big then MAX_API_COUNTS*/
#endif
};


/*
 * libev default loop, with a accept_watcher to accept the new connect
 * and dispatch to PORTER_PTHREAD.
 */
typedef struct {
	struct ev_io msmq_watcher;		/* msmq watcher for new message */

	unsigned int robin;
	pthread_t thread_id;			/* unique ID of this thread */
	
	struct ev_loop *loop;			/* libev loop this thread uses */
	struct ev_io accept_watcher;		/* accept watcher for new connect */
	struct ev_timer update_watcher;
	struct ev_signal signal_watcher;
} LEADER_PTHREAD;

/*
 * Each libev instance has a async_watcher, which other threads
 * can use to signal that they've put a new connection on its queue.
 */
typedef struct {
	int index;
	unsigned int depth;
	pthread_t thread_id;			/* unique ID of this thread */
	union virtual_system VMS;
	struct swift_task_node task;
	struct queue_list qlist;		/* queue of new request to handle *///用链表,互斥锁实现的普通安全队列
	struct supex_task_list tlist;		/* use prepare_watcher to do manage task *///用数组实现的无锁队列
	void *mount;		//用来指向mount_info链表，链表中每个结点为指向－组parter线程
	
	struct ev_loop *loop;			/* libev loop this thread uses */
#ifdef USE_PIPE
	int pfds[2];
	struct ev_io pipe_watcher;		/* accept watcher for new connect */
#else
	struct ev_async async_watcher;		/* async watcher for new connect */
#endif
	struct ev_prepare prepare_watcher;
	struct ev_idle idle_watcher;
	struct ev_check check_watcher;
} PORTER_PTHREAD;


/*******************************************/
int swift_mount(struct swift_cfg_list *conf);

int swift_start(void);



typedef int (*SWIFT_VMS_FCB)( lua_State **L, int last, struct swift_task_node *task );

int swift_for_alone_vm( void *W, SWIFT_VMS_FCB vms_fcb );
