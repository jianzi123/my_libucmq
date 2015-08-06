#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>		/* For mode constants */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "swift_api.h"
#include "swift_evcb.h"
#include "share_evcb.h"

int G_PORTER_COUNTS = 0;
int g_swift_init_done = false;

/******************************open************************************/
struct swift_settings g_swift_settings = {};

LEADER_PTHREAD g_leader_pthread = {};
PORTER_PTHREAD *g_porter_pthread = NULL;
static struct safe_once_init g_swift_mount_mark = {};
static struct safe_once_init g_first_init_mark = {};
/****************************function**********************************/
static void cfg_check(struct swift_cfg_list *conf)
{
	assert( conf->file_info.porter_counts );
}
#ifdef USE_HTTP_PROTOCOL
static void http_api_init( struct api_list *list, struct swift_cfg_list *conf )
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

struct api_list *swift_http_api_lookup(const char *api, int len)
{
	int idx = 0;
	struct api_list *p_api_list = g_swift_settings.apis;
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
static void redis_cmd_init( struct cmd_list *list, struct swift_cfg_list *conf )
{
	memset( list, 0, sizeof(struct cmd_list)*(MAX_CMD_COUNTS + 1) );
	int idx = 0;
	int max = MIN( LIMIT_FUNC_ORDER, (MAX_CMD_COUNTS + 1) );
	for (idx = 0; idx < max; idx ++ ){
		list[idx].type = conf->func_info[ idx ].type;
		list[idx].func = conf->func_info[ idx ].func;
	}
}
struct cmd_list *swift_redis_cmd_lookup(int order)
{
	int max = MIN( LIMIT_FUNC_ORDER, (MAX_CMD_COUNTS + 1) );
	if ((order < max) && (order >= 0)){
		return &g_swift_settings.cmds[ order ];
	}else{
		return NULL;
	}
}
#endif

static void first_init(void)
{
	SAFE_ONCE_INIT_COME( &g_first_init_mark );

	struct swift_cfg_file *p_cfg_file = &(g_swift_settings.conf->file_info);
	G_PORTER_COUNTS = p_cfg_file->porter_counts;

#ifdef USE_HTTP_PROTOCOL
	http_api_init( g_swift_settings.apis, g_swift_settings.conf );
#endif
#ifdef USE_REDIS_PROTOCOL
	redis_cmd_init( g_swift_settings.cmds, g_swift_settings.conf );
#endif
	//设置最大请求包的大小
	cache_peak( g_swift_settings.conf->file_info.max_req_size );

	init_log( p_cfg_file->log_path, p_cfg_file->log_file, p_cfg_file->log_level );
	open_new_log();

	SAFE_ONCE_INIT_OVER( &g_first_init_mark );
}


/*================================================================================*/
int swift_for_alone_vm( void *W, SWIFT_VMS_FCB vms_fcb )
{
	int error = 0;
	lua_State *PL = NULL;
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)W;

	swift_task_come( &p_porter->task.index, p_porter->task.id );

	error = vms_fcb( &p_porter->VMS.L, swift_task_last( &p_porter->task.index, p_porter->task.id ), &p_porter->task );
	if (error) {
		PL = p_porter->VMS.L;
		assert( PL );
		x_printf(E, "%s\n", lua_tostring( PL, -1 ));
		lua_pop( PL, 1 );
	}

	return error;
}


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
			
			struct swift_task_node task = {
				.id	= 0,
				.sfd	= 0,
				.type	= BIT8_TASK_TYPE_WHOLE,
				.origin	= BIT8_TASK_ORIGIN_MSMQ,
				.func	= g_swift_settings.conf->vmsys_cntl,
				.index	= 0,
				.data	= (void *)data
			};
			swift_all_task_hit( &task, false, 0 );
			break;
		default:
			x_printf(E, "UNKNOW CMD!\n");
	}

}




/*
 *函 数:init_porter
 *功 能:初始化porter线程,初始化porter中各种事件
 *参 数:
 *返回值:
 *说 明:
 */
static void init_porter(PORTER_PTHREAD *p_porter)
{
	/*init queue list*/
	//q_list是用单链表和互斥锁实现的安全队列
	cq_init( &(p_porter->qlist) );
	/*init task list*/
	//tlist是用数组实现的安全无锁队列
	supex_task_init( &(p_porter->tlist), sizeof(struct swift_task_node), MAX_SWIFT_QUEUE_NUMBER );

	/*set loop*/
	p_porter->loop = ev_loop_new (EVBACKEND_EPOLL | EVFLAG_NOENV);
#ifdef USE_PIPE
	/*create pipe*/
	if( pipe2(p_porter->pfds, (O_NONBLOCK | O_CLOEXEC)) < 0 ){
		x_perror("pipe2");
		exit(EXIT_FAILURE);
	}
#ifdef OPEN_OPTIMIZE
	fcntl(p_porter->pfds[0], F_SETPIPE_SZ, PIPE_MAX_SIZE);
	int size = fcntl(p_porter->pfds[0], F_GETPIPE_SZ, NULL);
	assert(size == PIPE_MAX_SIZE);
#endif
	printf("PIPE_MAX_SIZE is %d\n", PIPE_MAX_SIZE);

	/*set io_watcher*/
	p_porter->pipe_watcher.data = p_porter;
	ev_io_init( &(p_porter->pipe_watcher), swift_fetch_cb, p_porter->pfds[0], EV_READ);
	ev_io_start( p_porter->loop, &(p_porter->pipe_watcher) );
#else
	/* init async watcher */
	p_porter->async_watcher.data = p_porter;
	ev_async_init( &(p_porter->async_watcher), swift_async_cb);
	ev_async_start( p_porter->loop, &(p_porter->async_watcher) );/* Listen for notifications from other threads */
#endif
	/* init idle watcher */
	p_porter->idle_watcher.data = p_porter;
	ev_idle_init( &(p_porter->idle_watcher), swift_idle_cb);
	ev_idle_start( p_porter->loop, &(p_porter->idle_watcher) );
	/* init prepare watcher */
	p_porter->prepare_watcher.data = p_porter;
	ev_prepare_init( &(p_porter->prepare_watcher), swift_prepare_cb);
	ev_prepare_start( p_porter->loop, &(p_porter->prepare_watcher) );
	/* init check watcher */
	p_porter->check_watcher.data = p_porter;
	ev_check_init( &(p_porter->check_watcher), swift_check_cb);
	ev_check_start( p_porter->loop, &(p_porter->check_watcher) );
	return;
}

/*
 *函 数:start_porter
 *功 能:porter函数的入口函数
 */
static void *start_porter(void *arg)
{
	/* Any per-thread setup can happen here; thread_init() will block until
	 * all threads have finished initializing.
	 */
	struct safe_init_step *info = arg;
	SAFE_PTHREAD_INIT_COME( info );

	int idx = info->step;

	PORTER_PTHREAD *p_porter = &g_porter_pthread[ idx ];

	p_porter->index = idx;
	p_porter->thread_id = pthread_self();
	//初始化porter线程的各个事件
	init_porter( p_porter );
	if (g_swift_settings.conf->pthrd_init){
		//为porter线程创建parter线程
		g_swift_settings.conf->pthrd_init(p_porter);
	}

	SAFE_PTHREAD_INIT_OVER( info );
	/*start loop*/
	ev_loop(p_porter->loop, 0);
	/*exit*/
	ev_loop_destroy(p_porter->loop);
	return NULL;
}



/*
 *函 数:swift_mount
 *功 能:
 */
int swift_mount(struct swift_cfg_list *conf)
{
	SAFE_ONCE_INIT_COME( &g_swift_mount_mark );

	cfg_check(conf);
	g_swift_settings.conf = conf;

	SAFE_ONCE_INIT_OVER( &g_swift_mount_mark );
	return 0;
}

/*
 *函 数:swift_start
 *功 能:swift 的启动函数
 *参 数:
 *返回值:
 *说 明:
 */
int swift_start(void)
{
	/*init data*/
	//初始化日志等工作
	first_init();
	//初始化连接等工作
	pools_init();
	/*init proj*/
	if (g_swift_settings.conf->entry_init){
		g_swift_settings.conf->entry_init();
	}

	/*================*/
	g_porter_pthread = calloc( G_PORTER_COUNTS, sizeof(PORTER_PTHREAD) );
	assert( g_porter_pthread );
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start porter==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	safe_start_pthread((void *)start_porter, G_PORTER_COUNTS, NULL, NULL);


	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|================|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	struct swift_task_node task_init = {
		.id	= 0,
		.sfd	= 0,
		.type	= BIT8_TASK_TYPE_WHOLE,
		.origin	= BIT8_TASK_ORIGIN_INIT,
		.func	= g_swift_settings.conf->vmsys_init,
		.index	= 0,
		.data	= NULL
	};
	swift_all_task_hit( &task_init, true, 0 );
	g_swift_init_done = true;

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|================|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	/*init socket*/
	int listenfd = socket_init( g_swift_settings.conf->file_info.srv_port );

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start leader==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	g_leader_pthread.robin = 0;
	g_leader_pthread.thread_id = pthread_self();

	/*set loop*/
	g_leader_pthread.loop = ev_default_loop (0);

	/*register signal*/
	ev_signal_init( &(g_leader_pthread.signal_watcher), swift_signal_cb, SIGQUIT );//TODO
	ev_signal_start( g_leader_pthread.loop, &(g_leader_pthread.signal_watcher) );

	/*set msmq*/
	bool ok = msmq_init( g_swift_settings.conf->argv_info.msmq_name, shell_cntl );
	if (ok){
		ev_io_init( &(g_leader_pthread.msmq_watcher), msmq_share_cb, msmq_hand(), EV_READ );
		ev_io_start( g_leader_pthread.loop, &(g_leader_pthread.msmq_watcher) );
	}

	//ev_idle_init( &(g_leader_pthread.idle_watcher), idle_callback );
	//ev_idle_start( g_leader_pthread.loop, &(g_leader_pthread.idle_watcher) );

	/*set update timer*/
	ev_timer_init( &(g_leader_pthread.update_watcher), swift_update_cb, get_overplus_time(), 0. );
	ev_timer_start( g_leader_pthread.loop, &(g_leader_pthread.update_watcher) );

	/*set io_watcher*/
	ev_io_init( &(g_leader_pthread.accept_watcher), swift_accept_cb, listenfd, EV_READ );
	ev_io_start( g_leader_pthread.loop, &(g_leader_pthread.accept_watcher) );

	ev_loop(g_leader_pthread.loop, 0);
	ev_loop_destroy(g_leader_pthread.loop);
	if (ok){
		msmq_exit();
	}
	if (g_swift_settings.conf->vmsys_exit){
		//TODO
	}
	return 0;
}
