#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>		/* For mode constants */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "smart_api.h"
#include "smart_evcb.h"
#include "share_evcb.h"
#include "http.h"

int G_HANDER_COUNTS = 0;

int G_WORKER_COUNTS = 0;
int G_WORKER_COUNTS_TIMES = 0;
int G_MONITOR_TIMES = 0;  //监控定时器

#ifdef OPEN_CORO
int G_TASKER_COUNTS = 0;
int G_TASKER_COUNTS_TIMES = 0;
#endif

/*****************************extern***********************************/
#ifdef USE_HTTP_PROTOCOL
extern void http_response(struct data_node *p_node, int code);
#endif
#ifdef USE_REDIS_PROTOCOL
extern int rds_cmd_parse(struct data_node *p_node);
extern int rds_reset_parser(struct data_node *p_node);
#endif
/******************************open************************************/
struct smart_settings g_smart_settings = {};
MASTER_PTHREAD g_master_pthread = {};
RECVER_PTHREAD *g_recver_pthread = NULL;
SENDER_PTHREAD *g_sender_pthread = NULL;
WORKER_PTHREAD *g_worker_pthread = NULL;

static struct safe_once_init g_smart_mount_mark = {};
static struct safe_once_init g_first_init_mark = {};
/****************************function**********************************/
static void cfg_check(struct smart_cfg_list *conf)
{
	assert( conf->file_info.hander_counts
#ifdef OPEN_CORO
			&& conf->file_info.tasker_counts
#endif
			&& conf->file_info.worker_counts );
	
	if ( conf->file_info.worker_counts % conf->file_info.hander_counts ){
		x_printf(E, "worker_counts must is times of hander_counts!\n");
		exit(EXIT_FAILURE);
	}
#ifdef OPEN_CORO
	if ( conf->file_info.tasker_counts % conf->file_info.worker_counts ){
		x_printf(E, "tasker_counts must is times of worker_counts!\n");
		exit(EXIT_FAILURE);
	}
	printf(COLOR_RED "===========CORO_QUEUE_MIN_SLOT=%d CORO_QUEUE_ADD_STEP=%d==========\n" COLOR_NONE,
			CORO_QUEUE_MIN_SLOT, CORO_QUEUE_ADD_STEP);
	if ( (conf->file_info.tasker_counts / conf->file_info.worker_counts) < CORO_QUEUE_MIN_SLOT ){
		x_printf(E, "tasker_counts / worker_counts must equal or over than CORO_QUEUE_MIN_SLOT!\n");
		exit(EXIT_FAILURE);
	}
	#ifdef CORO_USE_LEAST_SPACE
	printf(COLOR_RED "===========MAX STACK ALLOW SIZE = %d.%dG==========\n" COLOR_NONE,
			(STACK_SIZE / 1024 / 1024) * conf->file_info.worker_counts / 1024,
	     (STACK_SIZE / 1024 / 1024) * conf->file_info.worker_counts % 1024 );
	#else
	printf(COLOR_RED "===========MAX STACK ALLOW SIZE = %d.%dG==========\n" COLOR_NONE,
			(STACK_SIZE / 1024 / 1024) * conf->file_info.tasker_counts / 1024,
	     (STACK_SIZE / 1024 / 1024) * conf->file_info.tasker_counts % 1024 );
	#endif

#endif
}
#ifdef USE_HTTP_PROTOCOL
static void http_api_init( struct api_list *list, struct smart_cfg_list *conf )
{
	memset( list, 0, sizeof(struct api_list)*(MAX_API_COUNTS + 1) );
	int idx = 0;
	if (conf->file_info.api_apply){
		list[idx].type = conf->func_info[ APPLY_FUNC_ORDER ].type;
		list[idx].name = conf->file_info.api_apply;
		list[idx].len = strlen( conf->file_info.api_apply );
		list[idx].func = conf->func_info[ APPLY_FUNC_ORDER ].func;
		idx ++;
	}
	if (conf->file_info.api_fetch){
		list[idx].type = conf->func_info[ FETCH_FUNC_ORDER ].type;
		list[idx].name = conf->file_info.api_fetch;
		list[idx].len = strlen( conf->file_info.api_fetch );
		list[idx].func = conf->func_info[ FETCH_FUNC_ORDER ].func;
		idx ++;
	}
	if (conf->file_info.api_merge){
		list[idx].type = conf->func_info[ MERGE_FUNC_ORDER ].type;
		list[idx].name = conf->file_info.api_merge;
		list[idx].len = strlen( conf->file_info.api_merge );
		list[idx].func = conf->func_info[ MERGE_FUNC_ORDER ].func;
		idx ++;
	}
	
	int i = 0;
	int custom = conf->file_info.api_counts - idx;
	for(i=0; i < custom; i++){
		list[idx + i].type = conf->func_info[ CUSTOM_FUNC_ORDER ].type;
		list[idx + i].name = &conf->file_info.api_names[i][0];
		list[idx + i].len = strlen( &conf->file_info.api_names[i][0] );
		list[idx + i].func = conf->func_info[ CUSTOM_FUNC_ORDER ].func;
	}
}

struct api_list *smart_http_api_lookup(const char *api, int len)
{
	int idx = 0;
	struct api_list *p_api_list = g_smart_settings.apis;
	for(idx = 0; p_api_list[idx].name != NULL; idx++){
		x_printf(D, "%s|   <===>   |%s\n", p_api_list[idx].name, api);
		if( !strncmp( p_api_list[idx].name, api, MAX(len, p_api_list[idx].len) ) ){
			return &p_api_list[idx];
		}
	}
	return NULL;
}
#endif
#ifdef USE_REDIS_PROTOCOL
static void redis_cmd_init( struct cmd_list *list, struct smart_cfg_list *conf )
{
	memset( list, 0, sizeof(struct cmd_list)*(MAX_CMD_COUNTS + 1) );
	int idx = 0;
	int max = MIN( LIMIT_FUNC_ORDER, (MAX_CMD_COUNTS + 1) );
	for (idx = 0; idx < max; idx ++ ){
		list[idx].type = conf->func_info[ idx ].type;
		list[idx].func = conf->func_info[ idx ].func;
	}
}
struct cmd_list *smart_redis_cmd_lookup(int order)
{
	int max = MIN( LIMIT_FUNC_ORDER, (MAX_CMD_COUNTS + 1) );
	if ((order < max) && (order >= 0)){
		return &g_smart_settings.cmds[ order ];
	}else{
		return NULL;
	}
}
#endif

static void first_init(void)
{
	SAFE_ONCE_INIT_COME( &g_first_init_mark );

	struct smart_cfg_file *p_cfg_file = &(g_smart_settings.conf->file_info);
	G_HANDER_COUNTS = p_cfg_file->hander_counts;
	G_WORKER_COUNTS = p_cfg_file->worker_counts;
	G_WORKER_COUNTS_TIMES = G_WORKER_COUNTS / G_HANDER_COUNTS;
        G_MONITOR_TIMES = (p_cfg_file->monitor_times > 0)?p_cfg_file->monitor_times : 1;
#ifdef OPEN_CORO
	G_TASKER_COUNTS = p_cfg_file->tasker_counts;
	G_TASKER_COUNTS_TIMES = G_TASKER_COUNTS / G_WORKER_COUNTS;
#endif

#ifdef USE_HTTP_PROTOCOL
	http_api_init( g_smart_settings.apis, g_smart_settings.conf );
#endif
#ifdef USE_REDIS_PROTOCOL
	redis_cmd_init( g_smart_settings.cmds, g_smart_settings.conf );
#endif
	cache_peak( g_smart_settings.conf->file_info.max_req_size );

	init_log( p_cfg_file->log_path, p_cfg_file->log_file, p_cfg_file->log_level );
	open_new_log();

	SAFE_ONCE_INIT_OVER( &g_first_init_mark );
}


/*================================================================================*/
#ifdef OPEN_CORO
int smart_for_alone_vm( void *user, void *task, int step, SMART_VMS_FCB vms_fcb )
{
	int error = 0;
	lua_State *PL = NULL;
	WORKER_PTHREAD *p_worker = (WORKER_PTHREAD *)user;
	struct smart_task_node *p_task = (struct smart_task_node *)task;

	smart_task_come( &p_task->index, p_task->id );

	error = vms_fcb( &p_worker->coro.VMS[ step ].L, smart_task_last( &p_task->index, p_task->id ),
			p_task, (long)&p_worker->coro.S );
	if (error) {
		PL = p_worker->coro.VMS[ step ].L;
		assert( PL );
		x_printf(E, "%s\n", lua_tostring( PL, -1 ));
		lua_pop( PL, 1 );
	}

	return error;
}

int smart_for_batch_vm( void *user, void *task, int step, SMART_VMS_FCB vms_fcb )
{
	int error = 0;
	lua_State *PL = NULL;
	WORKER_PTHREAD *p_worker = (WORKER_PTHREAD *)user;
	struct smart_task_node *p_task = (struct smart_task_node *)task;

	assert( p_worker->coro.S.nubs && p_worker->airing );
	while ( p_worker->coro.S.nubs != p_worker->airing ){
		coroutine_switch( &p_worker->coro.S );
	}
	
	int idx = 0;
	for (; idx < G_TASKER_COUNTS_TIMES; idx++) {
		smart_task_come( &p_task->index, p_task->id );

		error = vms_fcb( &p_worker->coro.VMS[ idx ].L, smart_task_last( &p_task->index, p_task->id ),
				p_task, (long)&p_worker->coro.S );
		if (error) {
			PL = p_worker->coro.VMS[ idx ].L;
			assert( PL );
			x_printf(E, "%s\n", lua_tostring( PL, -1 ));
			lua_pop( PL, 1 );
		}
	}
	
	p_worker->airing --;//FIXME: to move

	return error;
}
#endif
#ifdef OPEN_LINE
int smart_for_alone_vm( void *user, void *task, SMART_VMS_FCB vms_fcb )
{
	int error = 0;
	lua_State *PL = NULL;
	WORKER_PTHREAD *p_worker = (WORKER_PTHREAD *)user;
	struct smart_task_node *p_task = (struct smart_task_node *)task;

	smart_task_come( &p_task->index, p_task->id );

	error = vms_fcb( &p_worker->line.VMS.L, smart_task_last( &p_task->index, p_task->id ), p_task );
	if (error) {
		PL = p_worker->line.VMS.L;
		assert( PL );
		x_printf(E, "%s\n", lua_tostring( PL, -1 ));
		lua_pop( PL, 1 );
	}

	return error;
}
#endif
#ifdef OPEN_EVUV
int smart_for_alone_vm( void *user, void *task, SMART_VMS_FCB vms_fcb )
{
	int error = 0;
	lua_State *PL = NULL;
	WORKER_PTHREAD *p_worker = (WORKER_PTHREAD *)user;
	struct smart_task_node *p_task = (struct smart_task_node *)task;

	smart_task_come( &p_task->index, p_task->id );

	error = vms_fcb( &p_worker->evuv.VMS.L, smart_task_last( &p_task->index, p_task->id ), p_task );
	if (error) {
		PL = p_worker->evuv.VMS.L;
		assert( PL );
		x_printf(E, "%s\n", lua_tostring( PL, -1 ));
		lua_pop( PL, 1 );
	}

	return error;
}
#endif


/**********************************SERVER******************************************/

static void shell_cntl(const char *data)
{
	struct msg_info *msg = (struct msg_info *)data;
	switch (msg->opt){
		case 'l':/*load*/
		case 'f':/*free*/
		case 'o':/*open*/
		case 'c':/*close*/
		case 'd':/*delete*/
			;
			struct msg_info *data = (struct msg_info *)malloc(sizeof( struct msg_info ));
			assert( data );
			memcpy( data, msg, sizeof( struct msg_info ) );
			
			struct smart_task_node task = {
				.id	= 0,
				.sfd	= 0,
				.type	= BIT8_TASK_TYPE_WHOLE,
				.origin	= BIT8_TASK_ORIGIN_MSMQ,
				.func	= g_smart_settings.conf->vmsys_cntl,
				.index	= 0,
				.data	= (void *)data
			};
			smart_all_task_hit( &task, false, 0 );
			break;
		default:
			x_printf(E, "UNKNOW CMD!\n");
	}

}

#ifdef OPEN_CORO
static bool smart_safe_filter(void *user, void *task)
{
	WORKER_PTHREAD *p_worker = (WORKER_PTHREAD *)user;
	/*do manage task, one cpu execute,so don't need atomic or lock.*/
	bool ok = ( p_worker->airing > 0 )? true : false;
	if (false == ok){
		if (((struct smart_task_node *)task)->type == BIT8_TASK_TYPE_WHOLE){
			p_worker->airing ++;
		}
	}
	return ok;
}

static void *smart_task_handle(struct schedule *S, void *step)
{
	struct supex_coro *coro = S->main.data;
	int idx_task = (uintptr_t)step;

	/*
	 * WORKER_PTHREAD *p_worker = coro->data;
	 * int idx_work = p_worker->index;
	 * int idx_hand = idx_work / G_WORKER_COUNTS_TIMES;
	 * x_printf(D, "tasker id %d worker id %d hand id %d\n", idx_task, idx_work, idx_hand );
	 * RECVER_PTHREAD *p_recver = &g_recver_pthread[ idx_hand ];
	 * SENDER_PTHREAD *p_sender = &g_sender_pthread[ idx_hand ];
	 */

	struct smart_task_node *p_task = &((struct smart_task_node *)coro->task)[ idx_task ];
	/*call api*/
	x_printf(D, "tasker func =======1========= %d %p\n", idx_task, coro->C[ idx_task ].func);
	/*
	 * one bug:
	 * when stack_size of C.stack is too small, g_tasker_uthread[1] run will stack overflow, and affect g_tasker_uthread[0] data change.
	 */
	if ( p_task->func == NULL ){
		x_printf(S, "TASK is NULL function!\n");
		return NULL;
	}
	int err = p_task->func( coro->data, p_task, idx_task );//FIXME:change
	x_printf(D, "tasker func =======2========= %d %p\n", idx_task, coro->C[ idx_task ].func);

	if ( 0 == smart_task_over( p_task->id ) ){
		if (p_task->deal){
			*(p_task->deal) = TASK_IS_FINISH;
		}
		if (p_task->data != NULL){
			switch (p_task->origin){
				case BIT8_TASK_ORIGIN_MSMQ:
					free(p_task->data);
					break;
				default:
					break;
			}
			p_task->data = NULL;
		}
#ifdef USE_HTTP_PROTOCOL
		if ( p_task->origin == BIT8_TASK_ORIGIN_HTTP ){
			struct data_node *p_node = get_pool_addr( p_task->sfd );
			/*check error*/
			if (err) {
				http_response(p_node, 500);
				p_node->control = X_EXECUTE_ERROR;
			}else{
				http_response(p_node, 200);
				p_node->control = X_DONE_OK;
			}
			cache_free( &p_node->recv );
			smart_dispatch_task(SEND_TASK_TYPE, p_task->sfd);
			x_printf(D, "start to send back data!\n");
		}
#endif
#ifdef USE_REDIS_PROTOCOL
		if ( p_task->origin == BIT8_TASK_ORIGIN_REDIS ){
			struct data_node *p_node = get_pool_addr( p_task->sfd );
			/*check error*/
			if (err) {
				p_node->control = X_EXECUTE_ERROR;
			}else{
				p_node->control = X_DONE_OK;
			}
			cache_free( &p_node->recv );
			smart_dispatch_task(SEND_TASK_TYPE, p_task->sfd);
			x_printf(D, "start to send back data!\n");
		}
#endif
	}
	return NULL;
}
#endif
#if defined(OPEN_LINE) || defined(OPEN_EVUV)
static void smart_task_handle(void *data, void *addr)
{
	struct smart_task_node *p_task = addr;

	if ( p_task->func == NULL ){
		x_printf(S, "TASK is NULL function!\n");
		return;
	}
	/*call api*/
	int err = p_task->func( data, addr );//FIXME: add more args
	
	if ( 0 == smart_task_over( p_task->id ) ){
		if (p_task->deal){
			*(p_task->deal) = TASK_IS_FINISH;
		}
		if (p_task->data != NULL){
			switch (p_task->origin){
				case BIT8_TASK_ORIGIN_MSMQ:
					free(p_task->data);
					break;
				default:
					break;
			}
			p_task->data = NULL;
		}
#ifdef USE_HTTP_PROTOCOL
		if ( p_task->origin == BIT8_TASK_ORIGIN_HTTP ){
			struct data_node *p_node = get_pool_addr( p_task->sfd );
			/*check error*/
			if (err) {
				http_response(p_node, 500);
				p_node->control = X_EXECUTE_ERROR;
			}else{
				http_response(p_node, 200);
				p_node->control = X_DONE_OK;
			}
			cache_free( &p_node->recv );
			smart_dispatch_task(SEND_TASK_TYPE, p_task->sfd);
			x_printf(D, "start to send back data!\n");
		}
#endif
#ifdef USE_REDIS_PROTOCOL
		if ( p_task->origin == BIT8_TASK_ORIGIN_REDIS ){
			struct data_node *p_node = get_pool_addr( p_task->sfd );
			/*check error*/
			if (err) {
				p_node->control = X_EXECUTE_ERROR;
			}else{
				p_node->control = X_DONE_OK;
			}
			cache_free( &p_node->recv );
			smart_dispatch_task(SEND_TASK_TYPE, p_task->sfd);
			x_printf(D, "start to send back data!\n");
		}
#endif
	}
	return;
}
#endif


static void *start_worker(void *arg)
{
	/* Any per-thread setup can happen here; thread_init() will block until
	 * all threads have finished initializing.
	 */
	struct safe_init_step *info = arg;
	SAFE_PTHREAD_INIT_COME( info );

	int idx = (uintptr_t)info->step;

	WORKER_PTHREAD *p_worker = &g_worker_pthread[ idx ];

	p_worker->index = idx;
	p_worker->thread_id = pthread_self();
	supex_task_init(&p_worker->tlist, sizeof(struct smart_task_node), MAX_SMART_QUEUE_NUMBER);


	SAFE_PTHREAD_INIT_OVER( info );
#ifdef OPEN_CORO
	p_worker->coro.num = G_TASKER_COUNTS_TIMES;
	p_worker->coro.tsz = sizeof(struct smart_task_node);
	p_worker->coro.data = p_worker;
	p_worker->coro.task_lookup = g_smart_settings.conf->task_lookup;
	p_worker->coro.safe_filter = smart_safe_filter;
	p_worker->coro.task_handle = smart_task_handle;

	supex_coro_loop(&p_worker->coro);
#endif
#ifdef OPEN_LINE
	p_worker->line.tsz = sizeof(struct smart_task_node);
	p_worker->line.data = p_worker;
	p_worker->line.task_lookup = g_smart_settings.conf->task_lookup;
	p_worker->line.task_handle = smart_task_handle;

	supex_line_loop(&p_worker->line);
#endif
#ifdef OPEN_EVUV
	p_worker->evuv.tsz = sizeof(struct smart_task_node);
	p_worker->evuv.data = p_worker;
	p_worker->evuv.task_lookup = g_smart_settings.conf->task_lookup;
	p_worker->evuv.task_handle = smart_task_handle;

	supex_evuv_loop(&p_worker->evuv);
#endif
	return NULL;
}

static void init_hander(HANDER_PTHREAD *p_hander, int type)
{
	/*set owner*/
	p_hander->type = type;
	/*record pid*/
	p_hander->thread_id = pthread_self();
	/* init queue list */
	cq_init( &(p_hander->qlist) );

	/*set loop*/
	p_hander->loop = ev_loop_new (EVBACKEND_EPOLL | EVFLAG_NOENV);
#ifdef USE_PIPE
	/*create pipe*/
	if( pipe2(p_hander->pfds, (O_NONBLOCK | O_CLOEXEC)) < 0 ){
		x_perror("pipe2");
		exit(EXIT_FAILURE);
	}
#ifdef OPEN_OPTIMIZE
	fcntl(p_hander->pfds[0], F_SETPIPE_SZ, PIPE_MAX_SIZE);
	int size = fcntl(p_hander->pfds[0], F_GETPIPE_SZ, NULL);
	assert(size == PIPE_MAX_SIZE);
#endif
	printf("PIPE_MAX_SIZE is %d\n", PIPE_MAX_SIZE);

	/*set io_watcher*/
	p_hander->pipe_watcher.data = p_hander;
	ev_io_init( &(p_hander->pipe_watcher), smart_fetch_cb, p_hander->pfds[0], EV_READ);
	ev_io_start(p_hander->loop, &(p_hander->pipe_watcher) );
#else
	/* init async watcher */
	p_hander->async_watcher.data = p_hander;
	ev_async_init( &(p_hander->async_watcher), smart_async_cb);
	ev_async_start(p_hander->loop, &(p_hander->async_watcher) );/* Listen for notifications from other threads */
#endif
#if 0
	/* init prepare watcher */
	p_hander->prepare_watcher.data = p_hander;
	ev_prepare_init( &(p_hander->prepare_watcher), smart_prepare_cb);
	ev_prepare_start(p_hander->loop, &(p_hander->prepare_watcher) );
#endif
	/* init check watcher */
	p_hander->check_watcher.data = p_hander;
	ev_check_init( &(p_hander->check_watcher), smart_check_cb);
	ev_check_start(p_hander->loop, &(p_hander->check_watcher) );
	return;
}

static void *start_recver(void *arg)
{
	/* Any per-thread setup can happen here; thread_init() will block until
	 * all threads have finished initializing.
	 */
	struct safe_init_step *info = arg;
	SAFE_PTHREAD_INIT_COME( info );

	int idx = (uintptr_t)info->step;

	RECVER_PTHREAD *p_recver = &g_recver_pthread[ idx ];

	p_recver->robin = 0;
	p_recver->index = idx;

	init_hander( &p_recver->base, RECV_TASK_TYPE );

	SAFE_PTHREAD_INIT_OVER( info );
	/*start loop*/
	ev_loop(p_recver->base.loop, 0);
	/*exit*/
	ev_loop_destroy(p_recver->base.loop);
	return NULL;
}

static void *start_sender(void *arg)
{
	/* Any per-thread setup can happen here; thread_init() will block until
	 * all threads have finished initializing.
	 */
	struct safe_init_step *info = arg;
	SAFE_PTHREAD_INIT_COME( info );

	int idx = (uintptr_t)info->step;

	SENDER_PTHREAD *p_sender = &g_sender_pthread[ idx ];

	init_hander( &p_sender->base, SEND_TASK_TYPE );

	SAFE_PTHREAD_INIT_OVER( info );
	/*start loop*/
	ev_loop(p_sender->base.loop, 0);
	/*exit*/
	ev_loop_destroy(p_sender->base.loop);
	return NULL;
}


static bool smart_task_report(void *user, void *task)
{
	return supex_task_push(&((WORKER_PTHREAD *)user)->tlist, task);
}
static bool smart_task_lookup(void *user, void *task)
{
	return supex_task_pull(&((WORKER_PTHREAD *)user)->tlist, task);
}


int smart_mount(struct smart_cfg_list *conf)
{
	SAFE_ONCE_INIT_COME( &g_smart_mount_mark );

	cfg_check(conf);
	g_smart_settings.conf = conf;
	if ( !g_smart_settings.conf->task_lookup ){
		g_smart_settings.conf->task_lookup = smart_task_lookup;
	}
	if ( !g_smart_settings.conf->task_report ){
		g_smart_settings.conf->task_report = smart_task_report;
	}

	SAFE_ONCE_INIT_OVER( &g_smart_mount_mark );
	return 0;
}

int smart_start(void)
{
	/*init data*/
	first_init();
	pools_init();
	/*init proj*/
	if (g_smart_settings.conf->entry_init){
		g_smart_settings.conf->entry_init();
	}

	/*================*/
	g_worker_pthread = calloc( G_WORKER_COUNTS, sizeof(WORKER_PTHREAD) );
	assert( g_worker_pthread );

	g_recver_pthread = calloc( G_HANDER_COUNTS, sizeof(RECVER_PTHREAD) );
	assert( g_recver_pthread );

	g_sender_pthread = calloc( G_HANDER_COUNTS, sizeof(SENDER_PTHREAD) );
	assert( g_sender_pthread );
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start worker==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	safe_start_pthread((void *)start_worker, G_WORKER_COUNTS, NULL, NULL);

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|================|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	struct smart_task_node task_init = {
		.id	= 0,
		.sfd	= 0,
		.type	= BIT8_TASK_TYPE_WHOLE,
		.origin	= BIT8_TASK_ORIGIN_INIT,
		.func	= g_smart_settings.conf->vmsys_init,
		.index	= 0,
		.data	= NULL
	};
	smart_all_task_hit( &task_init, true, 0 );

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start hander==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	/*init socket*/
	int listenfd = socket_init( g_smart_settings.conf->file_info.srv_port );

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start recver==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	safe_start_pthread((void *)start_recver, G_HANDER_COUNTS, NULL, NULL);

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start sender==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	safe_start_pthread((void *)start_sender, G_HANDER_COUNTS, NULL, NULL);

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start master==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	g_master_pthread.robin = 0;
	g_master_pthread.thread_id = pthread_self();

	/*set loop*/
	g_master_pthread.loop = ev_default_loop (0);

	/*register signal*/
	ev_signal_init( &(g_master_pthread.signal_watcher), smart_signal_cb, SIGQUIT );//TODO
	ev_signal_start( g_master_pthread.loop, &(g_master_pthread.signal_watcher) );

	/*set msmq*/
	bool ok = msmq_init( g_smart_settings.conf->argv_info.msmq_name, shell_cntl );//FIXME
	if (ok){
		ev_io_init( &(g_master_pthread.msmq_watcher), msmq_share_cb, msmq_hand(), EV_READ );
		ev_io_start( g_master_pthread.loop, &(g_master_pthread.msmq_watcher) );
	}

	//ev_prepare_init( &(g_master_pthread.prepare_watcher), smart_prepare_cb );
	//ev_prepare_start( g_master_pthread.loop, &(g_master_pthread.prepare_watcher) );
	//ev_idle_init( &(g_master_pthread.idle_watcher), idle_callback );
	//ev_idle_start( g_master_pthread.loop, &(g_master_pthread.idle_watcher) );

	/*set update timer*/
	ev_timer_init( &(g_master_pthread.update_watcher), smart_update_cb, get_overplus_time(), 0. );
	ev_timer_start( g_master_pthread.loop, &(g_master_pthread.update_watcher) );
        
	/*set io_watcher*/
	ev_io_init( &(g_master_pthread.accept_watcher), smart_accept_cb, listenfd, EV_READ );
	ev_io_start( g_master_pthread.loop, &(g_master_pthread.accept_watcher) );

         //初始化监控,检查是否有回调函数
        if(NULL != g_smart_settings.conf->vmsys_monitor){
            ev_periodic_init(&(g_master_pthread.monitor_watcher), smart_monitor_cb, 0, G_MONITOR_TIMES, 0);
            ev_periodic_start(g_master_pthread.loop, &(g_master_pthread.monitor_watcher));
        }


	ev_loop(g_master_pthread.loop, 0);
	ev_loop_destroy(g_master_pthread.loop);
	if (ok){
		msmq_exit();
	}
	if (g_smart_settings.conf->vmsys_exit){
#ifdef OPEN_CORO
		//TODO
#else
		//TODO
#endif
	}
	return 0;
}
