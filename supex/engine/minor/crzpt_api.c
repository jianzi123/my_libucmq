/*
* a simple server use libev
*/
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>

#include "crzpt_api.h"
#include "crzpt_plan.h"
#include "crzpt_evcb.h"
#include "share_evcb.h"
/******************************open************************************/
int G_ROAMER_COUNTS = 0;
#ifdef OPEN_CORO
int G_PAUPER_COUNTS = 0;
int G_PAUPER_COUNTS_TIMES = 0;
#endif

struct crzpt_settings g_crzpt_settings = {};
FEEDER_PTHREAD g_feeder_pthread = {};				/* send task */
ROAMER_PTHREAD *g_roamer_pthread = NULL;				/* do task */

static struct safe_once_init g_crzpt_mount_mark = {};
static struct safe_once_init g_first_init_mark = {};
/****************************function**********************************/
static void cfg_check(struct crzpt_cfg_list *conf)
{
	assert( conf->file_info.roamer_counts );
#ifdef OPEN_CORO
	assert( conf->file_info.pauper_counts );
	if ( conf->file_info.pauper_counts % conf->file_info.roamer_counts ){
		x_printf(E, "pauper_counts must is times of roamer_counts!\n");
		exit(EXIT_FAILURE);
	}
	printf(COLOR_RED "===========CORO_QUEUE_MIN_SLOT=%d CORO_QUEUE_ADD_STEP=%d==========\n" COLOR_NONE,
			CORO_QUEUE_MIN_SLOT, CORO_QUEUE_ADD_STEP);
	if ( (conf->file_info.pauper_counts / conf->file_info.roamer_counts) < CORO_QUEUE_MIN_SLOT ){
		x_printf(E, "pauper_counts / roamer_counts) must equal or over then CORO_QUEUE_MIN_SLOT!\n");
		exit(EXIT_FAILURE);
	}
	#ifdef CORO_USE_LEAST_SPACE
	printf(COLOR_RED "===========MAX STACK ALLOW SIZE = %d.%dG==========\n" COLOR_NONE,
			(STACK_SIZE / 1024 / 1024) * conf->file_info.roamer_counts / 1024,
	     (STACK_SIZE / 1024 / 1024) * conf->file_info.roamer_counts % 1024 );
	#else
	printf(COLOR_RED "===========MAX STACK ALLOW SIZE = %d.%dG==========\n" COLOR_NONE,
			(STACK_SIZE / 1024 / 1024) * conf->file_info.pauper_counts / 1024,
	     (STACK_SIZE / 1024 / 1024) * conf->file_info.pauper_counts % 1024 );
	#endif
#endif
}

static void first_init(void)
{
	SAFE_ONCE_INIT_COME( &g_first_init_mark );

	struct crzpt_cfg_file *p_cfg_file = &(g_crzpt_settings.conf->file_info);
	G_ROAMER_COUNTS = p_cfg_file->roamer_counts;
#ifdef OPEN_CORO
	G_PAUPER_COUNTS = p_cfg_file->pauper_counts;
	G_PAUPER_COUNTS_TIMES = G_PAUPER_COUNTS / G_ROAMER_COUNTS;
#endif

	init_log( p_cfg_file->log_path, p_cfg_file->log_file, p_cfg_file->log_level );
	open_new_log();
	
	crzpt_plan_list_init( );

	g_crzpt_settings.conf->store_firing("crzptDB", 32*1024, 64*1024*1024, 64*1024*1024, 10);//TODO

	SAFE_ONCE_INIT_OVER( &g_first_init_mark );
}



/********************************************************/
#ifdef OPEN_CORO
int crzpt_for_alone_vm( void *user, void *task, int step, CRZPT_VMS_FCB vms_fcb )
{
	int error = 0;
	lua_State *PL = NULL;
	ROAMER_PTHREAD *p_roamer = (ROAMER_PTHREAD *)user;
	struct crzpt_task_node *p_task = (struct crzpt_task_node *)task;

	crzpt_task_come( &p_task->index, p_task->id );

	error = vms_fcb( &p_roamer->coro.VMS[ step ].L, crzpt_task_last( &p_task->index, p_task->id ),
			p_task, (long)&p_roamer->coro.S );
	if (error) {
		PL = p_roamer->coro.VMS[ step ].L;
		assert( PL );
		x_printf(E, "%s\n", lua_tostring( PL, -1 ));
		lua_pop( PL, 1 );
	}

	return error;
}

int crzpt_for_batch_vm( void *user, void *task, int step, CRZPT_VMS_FCB vms_fcb )
{
	int error = 0;
	lua_State *PL = NULL;
	ROAMER_PTHREAD *p_roamer = (ROAMER_PTHREAD *)user;
	struct crzpt_task_node *p_task = (struct crzpt_task_node *)task;

	assert( p_roamer->coro.S.nubs && p_roamer->airing );
	while ( p_roamer->coro.S.nubs != p_roamer->airing ){
		coroutine_switch( &p_roamer->coro.S );
	}
	
	int idx = 0;
	for (; idx < G_PAUPER_COUNTS_TIMES; idx++) {
		crzpt_task_come( &p_task->index, p_task->id );

		error = vms_fcb( &p_roamer->coro.VMS[ idx ].L, crzpt_task_last( &p_task->index, p_task->id ),
				p_task, (long)&p_roamer->coro.S );
		if (error) {
			PL = p_roamer->coro.VMS[ idx ].L;
			assert( PL );
			x_printf(E, "%s\n", lua_tostring( PL, -1 ));
			lua_pop( PL, 1 );
		}
	}
	
	p_roamer->airing --;//FIXME: to move

	return error;
}
#endif
#ifdef OPEN_LINE
int crzpt_for_alone_vm( void *user, void *task, CRZPT_VMS_FCB vms_fcb )
{
	int error = 0;
	lua_State *PL = NULL;
	ROAMER_PTHREAD *p_roamer = (ROAMER_PTHREAD *)user;
	struct crzpt_task_node *p_task = (struct crzpt_task_node *)task;

	crzpt_task_come( &p_task->index, p_task->id );

	error = vms_fcb( &p_roamer->line.VMS.L, crzpt_task_last( &p_task->index, p_task->id ), p_task );
	if (error) {
		PL = p_roamer->line.VMS.L;
		assert( PL );
		x_printf(E, "%s\n", lua_tostring( PL, -1 ));
		lua_pop( PL, 1 );
	}

	return error;
}
#endif
#ifdef OPEN_EVUV
int crzpt_for_alone_vm( void *user, void *task, CRZPT_VMS_FCB vms_fcb )
{
	int error = 0;
	lua_State *PL = NULL;
	ROAMER_PTHREAD *p_roamer = (ROAMER_PTHREAD *)user;
	struct crzpt_task_node *p_task = (struct crzpt_task_node *)task;

	crzpt_task_come( &p_task->index, p_task->id );

	error = vms_fcb( &p_roamer->evuv.VMS.L, crzpt_task_last( &p_task->index, p_task->id ), p_task );
	if (error) {
		PL = p_roamer->evuv.VMS.L;
		assert( PL );
		x_printf(E, "%s\n", lua_tostring( PL, -1 ));
		lua_pop( PL, 1 );
	}

	return error;
}
#endif
/********************************************************/


/*
#ifdef CRZPT_OPEN_MSMQ
				put_msmq_msg( MSMQ_NAME(0), "add task\n", 2);
#else
				res = put_fifo_msg("add task\n", 9);// when one pthread,it don't need lock
#endif
*/

/**********************************SERVER******************************************/
static void accept_cntl(const char *data)
{
	struct crzpt_task_node task = {};
	task.origin	= BIT8_TASK_ORIGIN_MSMQ;

	struct msg_info *msg = (struct msg_info *)data;
	struct msg_info *tmp = NULL;
	x_printf(D, "mode is %c", msg->mode);
	switch (msg->mode){
		case CRZPT_MSMQ_TYPE_PUSH:
			task.type	= BIT8_TASK_TYPE_WHOLE;
			task.func	= g_crzpt_settings.conf->vmsys_push;

			tmp = (struct msg_info *)malloc(sizeof( struct msg_info ));
			assert( tmp );
			memcpy( tmp, msg, sizeof( struct msg_info ) );
			task.data = (void *)tmp;

#ifdef CRZPT_MSMQ_SELECT_ASYNC
			crzpt_all_task_hit( &task, false );
#else
			crzpt_all_task_hit( &task, true );
#endif
			break;
		case CRZPT_MSMQ_TYPE_PULL:
			task.type	= BIT8_TASK_TYPE_ALONE;
			task.func	= g_crzpt_settings.conf->vmsys_pull;

			tmp = (struct msg_info *)malloc(sizeof( struct msg_info ));
			assert( tmp );
			memcpy( tmp, msg, sizeof( struct msg_info ) );
			task.data = (void *)tmp;

#ifdef CRZPT_MSMQ_SELECT_ASYNC
			crzpt_one_task_hit( &task, false );
#else
			crzpt_one_task_hit( &task, true );
#endif
			break;
		case CRZPT_MSMQ_TYPE_INSERT:
			crzpt_cpp_make_plan(msg->time, 1, msg->data);//TODO
			break;
		default:
			x_printf(E, "UNKNOW CMD [%c]!\n", msg->mode);
			return;
	}
}

#ifdef OPEN_CORO
static bool crzpt_safe_filter(void *user, void *task)
{
	ROAMER_PTHREAD *p_roamer = (ROAMER_PTHREAD *)user;
	/*do manage task, one cpu execute,so don't need atomic or lock.*/
	bool ok = ( p_roamer->airing > 0 )? true : false;
	if (false == ok){
		if (((struct crzpt_task_node *)task)->type == BIT8_TASK_TYPE_WHOLE){
			p_roamer->airing ++;
		}
	}
	return ok;
}

static void *crzpt_task_handle(struct schedule *S, void *step)
{
	struct supex_coro *coro = S->main.data;
	int idx_paup = (uintptr_t)step;

	/*
	 * ROAMER_PTHREAD *p_roamer = coro->data;
	 * int idx_roam = p_roamer->index;
	 */

	struct crzpt_task_node *p_task = &((struct crzpt_task_node *)coro->task)[ idx_paup ];
	/*call api*/
	x_printf(D, "pauper func =======1========= %d %p\n", idx_paup, coro->C[ idx_paup ].func);
	/*
	 * one bug:
	 * when stack_size of C.stack is too small, g_pauper_uthread[1] run will stack overflow, and affect g_pauper_uthread[0] data change.
	 */
	if ( p_task->func == NULL ){
		x_printf(S, "TASK is NULL function!\n");
		return NULL;
	}
	int err = p_task->func( coro->data, p_task, idx_paup );//FIXME:change
	x_printf(D, "pauper func =======2========= %d %p\n", idx_paup, coro->C[ idx_paup ].func);

	if ( 0 == crzpt_task_over( p_task->id ) ){
		x_printf(D, "task done over!\n");
		struct crzpt_plan_node *p_plan = (struct crzpt_plan_node *)p_task->data;
		if (p_task->deal){
			*(p_task->deal) = TASK_IS_FINISH;
		}
		if (p_task->data != NULL){
			switch (p_task->origin){
				case BIT8_TASK_ORIGIN_MSMQ:
					free(p_task->data);
					break;
				case BIT8_TASK_ORIGIN_TIME:
					if (p_plan->live == EXECUTE_DEATH){
						free(p_task->data);
					}
					break;
				default:
					break;
			}
			p_task->data = NULL;
		}
	}
	return NULL;
}
#endif

#if defined(OPEN_LINE) || defined(OPEN_EVUV)
static void crzpt_task_handle(void *data, void *addr)
{
	struct crzpt_task_node *p_task = addr;

	if ( p_task->func == NULL ){
		x_printf(S, "TASK is NULL function!\n");
		return;
	}
	/*call api*/
	int err = p_task->func( data, addr );//FIXME: add more args
	
	if ( 0 == crzpt_task_over( p_task->id ) ){
		x_printf(D, "task done over!\n");
		struct crzpt_plan_node *p_plan = (struct crzpt_plan_node *)p_task->data;
		if (p_task->deal){
			*(p_task->deal) = TASK_IS_FINISH;
		}
		if (p_task->data != NULL){
			switch (p_task->origin){
				case BIT8_TASK_ORIGIN_MSMQ:
					free(p_task->data);
					break;
				case BIT8_TASK_ORIGIN_TIME:
					if (p_plan->live == EXECUTE_DEATH){
						free(p_task->data);
					}
					break;
				default:
					break;
			}
			p_task->data = NULL;
		}
	}
	return;
}
#endif
static void *start_roamer(void *arg)
{
	/* Any per-thread setup can happen here; thread_init() will block until
	 * all threads have finished initializing.
	 */
	struct safe_init_step *info = arg;
	SAFE_PTHREAD_INIT_COME( info );

	int idx = (uintptr_t)info->step;

	ROAMER_PTHREAD *p_roamer = &g_roamer_pthread[ idx ];

	p_roamer->index = idx;
	p_roamer->thread_id = pthread_self();
	supex_task_init(&p_roamer->tlist, sizeof(struct crzpt_task_node), MAX_CRZPT_QUEUE_NUMBER);


	SAFE_PTHREAD_INIT_OVER( info );
#ifdef OPEN_CORO
	p_roamer->coro.num = G_PAUPER_COUNTS_TIMES;
	p_roamer->coro.tsz = sizeof(struct crzpt_task_node);
	p_roamer->coro.data = p_roamer;
	p_roamer->coro.task_lookup = g_crzpt_settings.conf->task_lookup;
	p_roamer->coro.safe_filter = crzpt_safe_filter;
	p_roamer->coro.task_handle = crzpt_task_handle;

	supex_coro_loop(&p_roamer->coro);
#endif
#ifdef OPEN_LINE
	p_roamer->line.tsz = sizeof(struct crzpt_task_node);
	p_roamer->line.data = p_roamer;
	p_roamer->line.task_lookup = g_crzpt_settings.conf->task_lookup;
	p_roamer->line.task_handle = crzpt_task_handle;

	supex_line_loop(&p_roamer->line);
#endif
#ifdef OPEN_EVUV
	p_roamer->evuv.tsz = sizeof(struct crzpt_task_node);
	p_roamer->evuv.data = p_roamer;
	p_roamer->evuv.task_lookup = g_crzpt_settings.conf->task_lookup;
	p_roamer->evuv.task_handle = crzpt_task_handle;

	supex_evuv_loop(&p_roamer->evuv);
#endif
	return NULL;
}

static bool crzpt_task_report(void *user, void *task)
{
	return supex_task_push(&((ROAMER_PTHREAD *)user)->tlist, task);
}
static bool crzpt_task_lookup(void *user, void *task)
{
	return supex_task_pull(&((ROAMER_PTHREAD *)user)->tlist, task);
}



int crzpt_mount(struct crzpt_cfg_list *conf)
{
	SAFE_ONCE_INIT_COME( &g_crzpt_mount_mark );

	cfg_check(conf);
	g_crzpt_settings.conf = conf;
	if ( !g_crzpt_settings.conf->task_lookup ){
		g_crzpt_settings.conf->task_lookup = crzpt_task_lookup;
	}
	if ( !g_crzpt_settings.conf->task_report ){
		g_crzpt_settings.conf->task_report = crzpt_task_report;
	}

	SAFE_ONCE_INIT_OVER( &g_crzpt_mount_mark );
	return 0;
}

int crzpt_start(void)
{
	/*init data*/
	first_init();
	/*init proj*/
	if (g_crzpt_settings.conf->entry_init){
		g_crzpt_settings.conf->entry_init();
	}

	/*================*/
	g_roamer_pthread = calloc( G_ROAMER_COUNTS, sizeof(ROAMER_PTHREAD) );
	assert( g_roamer_pthread );
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start roamer==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	safe_start_pthread((void *)start_roamer, G_ROAMER_COUNTS, NULL, NULL);

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|================|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	struct crzpt_task_node task_init = {
		.id	= 0,
		.sfd	= 0,
		.type	= BIT8_TASK_TYPE_WHOLE,
		.origin	= BIT8_TASK_ORIGIN_INIT,
		.func	= g_crzpt_settings.conf->vmsys_init,
		.index	= 0,
		.data	= NULL
	};
	crzpt_all_task_hit( &task_init, true );

	struct crzpt_task_node task_load = {
		.id	= 0,
		.sfd	= 0,
		.type	= BIT8_TASK_TYPE_ALONE,
		.origin	= BIT8_TASK_ORIGIN_INIT,
		.func	= g_crzpt_settings.conf->vmsys_load,
		.index	= 0,
		.data	= NULL
	};
	crzpt_one_task_hit( &task_load, true );

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start feeder==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	g_feeder_pthread.thread_id = pthread_self();

	/*set loop*/
	g_feeder_pthread.loop = ev_loop_new (EVBACKEND_EPOLL | EVFLAG_NOENV);

#ifdef CRZPT_OPEN_MSMQ
	/*set msmq*/
	bool ok = msmq_init( g_crzpt_settings.conf->argv_info.msmq_name, accept_cntl );
	if (ok){
		ev_io_init( &(g_feeder_pthread.msmq_watcher), msmq_share_cb, msmq_hand(), EV_READ );
		ev_io_start( g_feeder_pthread.loop, &(g_feeder_pthread.msmq_watcher) );
	}
#else
#if 0
	/*register signal QUIT*/
	ev_signal_init( &(g_feeder_pthread.signal_quit_watcher), crzpt_signal_cb, SIGQUIT );
	ev_signal_start( g_feeder_pthread.loop, &(g_feeder_pthread.signal_quit_watcher) );
	/*register signal ADD*/
	ev_signal_init( &(g_feeder_pthread.signal_add_watcher), crzpt_signal_cb, SIG_APP_ADD );
	ev_signal_start( g_feeder_pthread.loop, &(g_feeder_pthread.signal_add_watcher) );
	/*register signal SET*/
	ev_signal_init( &(g_feeder_pthread.signal_set_watcher), crzpt_signal_cb, SIG_APP_SET );
	ev_signal_start( g_feeder_pthread.loop, &(g_feeder_pthread.signal_set_watcher) );
	/*register signal DEL*/
	ev_signal_init( &(g_feeder_pthread.signal_del_watcher), crzpt_signal_cb, SIG_APP_DEL );
	ev_signal_start( g_feeder_pthread.loop, &(g_feeder_pthread.signal_del_watcher) );
#endif
#endif
	/*register idle*/
	ev_idle_init( &(g_feeder_pthread.idle_watcher), crzpt_idle_cb);
	ev_idle_start( g_feeder_pthread.loop, &(g_feeder_pthread.idle_watcher) );

	printf("start ...\n");
	ev_loop(g_feeder_pthread.loop, 0);
	ev_loop_destroy(g_feeder_pthread.loop);//if thread not break,other loop clean also at this ----> TODO test
#ifdef CRZPT_OPEN_MSMQ
	if (ok){
		msmq_exit();
	}
#else
#endif
	if (g_crzpt_settings.conf->vmsys_exit){
#ifdef OPEN_CORO
		//TODO
#else
		//TODO
#endif
	}
	return 0;
}
