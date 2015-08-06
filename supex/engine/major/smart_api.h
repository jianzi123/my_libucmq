#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "list.h"
#include "net_cache.h"
#include "utils.h"
#include "coro.h"
#include "smart_task.h"
#include "smart_cfg.h"
#include "supex.h"


struct smart_settings {
	struct smart_cfg_list *conf;
#ifdef USE_HTTP_PROTOCOL
	struct api_list apis[ MAX_API_COUNTS + 1 ];/*all api counts can't big then MAX_API_COUNTS*/
#endif
#ifdef USE_REDIS_PROTOCOL
	struct cmd_list cmds[ MAX_CMD_COUNTS + 1 ];/*all cmd counts can't big then MAX_API_COUNTS*/
#endif
};


/*
 * libev default loop, with a accept_watcher to accept the new connect
 * and dispatch to RECVER_PTHREAD.
 */
typedef struct {
	struct ev_io msmq_watcher;		/* msmq watcher for new message */

	unsigned int robin;
	pthread_t thread_id;			/* unique ID of this thread */
	
	struct ev_loop *loop;			/* libev loop this thread uses */
	struct ev_io accept_watcher;		/* accept watcher for new connect */
	struct ev_timer update_watcher;
        struct ev_periodic monitor_watcher;     //监控事件
	struct ev_signal signal_watcher;
} MASTER_PTHREAD;


typedef struct {
#ifdef OPEN_CORO
	struct supex_coro coro;
	int airing;				/*BIT8_TASK_TYPE_WHOLE task count*/
#endif
#ifdef OPEN_LINE
	struct supex_line line;
#endif
#ifdef OPEN_EVUV
	struct supex_evuv evuv;
#endif

	void *data;
	int batch;
	int index;
	pthread_t thread_id;			/* unique ID of this thread */
	struct supex_task_list tlist;
} WORKER_PTHREAD;

/*
 * Each libev instance has a async_watcher, which other threads
 * can use to signal that they've put a new connection on its queue.
 */
typedef struct {
	enum {
		RECV_TASK_TYPE = 0,
		SEND_TASK_TYPE = 1,
	} type;
	pthread_t thread_id;			/* unique ID of this thread */
	struct queue_list qlist;		/* queue of new request to handle */
	
	struct ev_loop *loop;			/* libev loop this thread uses */
#ifdef USE_PIPE
	int pfds[2];
	struct ev_io pipe_watcher;		/* accept watcher for new connect */
#else
	struct ev_async async_watcher;		/* async watcher for new connect */
#endif
	//struct ev_prepare prepare_watcher;
	struct ev_check check_watcher;
	//struct ev_idle idle_watcher;
} HANDER_PTHREAD;

typedef struct {
	HANDER_PTHREAD base;
	int index;
	unsigned int robin;
} RECVER_PTHREAD;

typedef struct {
	HANDER_PTHREAD base;
} SENDER_PTHREAD;

/*******************************************/
int smart_mount(struct smart_cfg_list *conf);

int smart_start(void);



#ifdef OPEN_CORO
typedef int (*SMART_VMS_FCB)( lua_State **L, int last, struct smart_task_node *task, long S );

int smart_for_alone_vm( void *user, void *task, int step, SMART_VMS_FCB vms_fcb );

int smart_for_batch_vm( void *user, void *task, int step, SMART_VMS_FCB vms_fcb );
#endif
#if defined(OPEN_LINE) || defined(OPEN_EVUV)
typedef int (*SMART_VMS_FCB)( lua_State **L, int last, struct smart_task_node *task );

int smart_for_alone_vm( void *user, void *task, SMART_VMS_FCB vms_fcb );
#endif
