#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "list.h"
#include "utils.h"
#include "crzpt_cfg.h"
#include "crzpt_task.h"
#include "supex.h"


#define SIG_APP_ADD	36
#define SIG_APP_SET	37
#define SIG_APP_DEL	38

struct crzpt_settings {
	struct crzpt_cfg_list *conf;
};

typedef struct {
	pthread_t thread_id;			/* unique ID of this thread */

	struct ev_loop *loop;			/* libev loop this thread uses */
	struct ev_idle idle_watcher;
#ifdef CRZPT_OPEN_MSMQ
	struct ev_io msmq_watcher;		/* msmq watcher for new message */
#else
#if 0
	struct ev_signal signal_quit_watcher;
	struct ev_signal signal_add_watcher;
	struct ev_signal signal_set_watcher;
	struct ev_signal signal_del_watcher;
#endif
#endif
} FEEDER_PTHREAD;

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
	//int batch;//FIXME
	int index;
	pthread_t thread_id;			/* unique ID of this thread */
	struct supex_task_list tlist;
} ROAMER_PTHREAD;

int crzpt_mount(struct crzpt_cfg_list *conf);

int crzpt_start(void);


#ifdef OPEN_CORO
typedef int (*CRZPT_VMS_FCB)( lua_State **L, int last, struct crzpt_task_node *task, long S );

int crzpt_for_alone_vm( void *user, void *task, int step, CRZPT_VMS_FCB vms_fcb );

int crzpt_for_batch_vm( void *user, void *task, int step, CRZPT_VMS_FCB vms_fcb );
#endif
#if defined(OPEN_LINE) || defined(OPEN_EVUV)
typedef int (*CRZPT_VMS_FCB)( lua_State **L, int last, struct crzpt_task_node *task );

int crzpt_for_alone_vm( void *user, void *task, CRZPT_VMS_FCB vms_fcb );
#endif
