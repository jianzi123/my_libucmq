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

#include "sniff_api.h"

struct sniff_bind_data {
	int batch;
	int genus;
	void *data;
};
/******************************open************************************/
int G_PARTER_COUNTS = 0;
#ifdef OPEN_CORO
int G_SHARER_COUNTS = 0;
int G_SHARER_COUNTS_TIMES = 0;
#endif

struct sniff_settings g_sniff_settings = {};

static struct safe_once_init g_sniff_mount_mark = {};
static struct safe_once_init g_first_init_mark = {};

static struct supex_task_list g_supex_task_list = {};	//全局的supex_task_list队列
/****************************function**********************************/
static void cfg_check(struct sniff_cfg_list *conf)
{
	assert( conf->file_info.parter_counts );
#ifdef OPEN_CORO
	assert( conf->file_info.sharer_counts );
	if ( conf->file_info.sharer_counts % conf->file_info.parter_counts ){
		x_printf(E, "sharer_counts must is times of parter_counts!\n");
		exit(EXIT_FAILURE);
	}
	printf(COLOR_RED "===========CORO_QUEUE_MIN_SLOT=%d CORO_QUEUE_ADD_STEP=%d==========\n" COLOR_NONE,
			CORO_QUEUE_MIN_SLOT, CORO_QUEUE_ADD_STEP);
	if ( (conf->file_info.sharer_counts / conf->file_info.parter_counts) < CORO_QUEUE_MIN_SLOT ){
		x_printf(E, "sharer_counts / parter_counts) must equal or over then CORO_QUEUE_MIN_SLOT!\n");
		exit(EXIT_FAILURE);
	}
	#ifdef CORO_USE_LEAST_SPACE
	printf(COLOR_RED "===========MAX STACK ALLOW SIZE = %d.%dG==========\n" COLOR_NONE,
			(STACK_SIZE / 1024 / 1024) * conf->file_info.parter_counts / 1024,
	     (STACK_SIZE / 1024 / 1024) * conf->file_info.parter_counts % 1024 );
	#else
	printf(COLOR_RED "===========MAX STACK ALLOW SIZE = %d.%dG==========\n" COLOR_NONE,
			(STACK_SIZE / 1024 / 1024) * conf->file_info.sharer_counts / 1024,
	     (STACK_SIZE / 1024 / 1024) * conf->file_info.sharer_counts % 1024 );
	#endif
#endif
}

static void first_init(void)
{
	SAFE_ONCE_INIT_COME( &g_first_init_mark );

	struct sniff_cfg_file *p_cfg_file = &(g_sniff_settings.conf->file_info);
	G_PARTER_COUNTS = p_cfg_file->parter_counts;
#ifdef OPEN_CORO
	G_SHARER_COUNTS = p_cfg_file->sharer_counts;
	G_SHARER_COUNTS_TIMES = G_SHARER_COUNTS / G_PARTER_COUNTS;
#endif

	init_log( p_cfg_file->log_path, p_cfg_file->log_file, p_cfg_file->log_level );
	open_new_log();
	
	SAFE_ONCE_INIT_OVER( &g_first_init_mark );
}



/********************************************************/
#ifdef OPEN_CORO
int sniff_for_alone_vm( void *user, void *task, int step, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err )
{
	int error = 0;
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)user;
	void **base = &p_parter->coro.VMS[ step ].base;

	error = vms_fcb( base, ((struct sniff_task_node *)task)->last, task, (long)&p_parter->coro.S );
	if (error) {
		vms_err( base );
	}
	return error;
}

int sniff_for_batch_vm( void *user, void *task, int step, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err )
{
	int error = 0;
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)user;
#if 0
	assert( p_parter->coro.S.nubs && p_parter->airing );
	while ( p_parter->coro.S.nubs != p_parter->airing ){
		coroutine_switch( &p_parter->coro.S );
	}
#endif
	int idx = 0;
	for (; idx < G_SHARER_COUNTS_TIMES; idx++) {
		void **base = &p_parter->coro.VMS[ idx ].base;

		bool last = (idx == step) ? ((struct sniff_task_node *)task)->last : false;
		error = vms_fcb( base, last, task, (long)&p_parter->coro.S );
		if (error) {
			vms_err( base );
		}
	}
#if 0
	p_parter->airing --;
#endif

	return error;
}
#endif

#ifdef OPEN_LINE
int sniff_for_alone_vm( void *user, void *task, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err )
{
	int error = 0;
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)user;
	void **base = &p_parter->line.VMS.base;

	error = vms_fcb( base, ((struct sniff_task_node *)task)->last, task );
	if (error) {
		vms_err( base );
	}
	return error;
}
#endif

#ifdef OPEN_EVUV
int sniff_for_alone_vm( void *user, void *task, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err )
{
	int error = 0;
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)user;
	void **base = &p_parter->evuv.VMS.base;

	error = vms_fcb( base, ((struct sniff_task_node *)task)->last, task );
	if (error) {
		vms_err( base );
	}
	return error;
}
#endif


/*
 *函 数:sniff_all_task_hit
 *功 能:向head指向的parter线程列表中的每一个parter线程添加task任务
 *参 数:head 指向第一个parter线程
 *返回值:
 *说 明:
 */
void sniff_all_task_hit( PARTER_PTHREAD *head, struct sniff_task_node *task )
{
	bool last = task->last;

	task->type	= BIT8_TASK_TYPE_WHOLE;

	PARTER_PTHREAD *temp = head->next;
	while( temp != head ){
		task->last = false;
		//向parter线程中添加任务(parter线程有一个用数组实现的无锁队列)
		g_sniff_settings.conf->task_report( temp, task );
		temp = temp->next;
	}
	task->last = last;
	g_sniff_settings.conf->task_report( head, task );
}


/*
 * 函 数:sniff_one_task_hit
 * 功 能:向某个parter线程中添加任务
 * 参 数:
 * 返回值:
 * 说 明:
 */
void sniff_one_task_hit( PARTER_PTHREAD *head, struct sniff_task_node *task )
{
#if 0
	unsigned int idx = get_array_random_index( G_PARTER_COUNTS );//is not effect

	task->type	= BIT8_TASK_TYPE_ALONE;

	PARTER_PTHREAD *temp = head;
	int i = 0;
	for (i = 0; i < idx; i++){
		temp = temp->next;
	}
	g_sniff_settings.conf->task_report( temp, task );
#else
	task->type	= BIT8_TASK_TYPE_ALONE;
	g_sniff_settings.conf->task_report( head, task );
#endif
}

/**********************************SERVER******************************************/
#ifdef OPEN_CORO
static void *sniff_task_handle(struct schedule *S, void *step)
{
	struct supex_coro *coro = S->main.data;
	int idx_task = (uintptr_t)step;

	//PARTER_PTHREAD *p_parter = coro->data;
	struct sniff_task_node *p_task = &((struct sniff_task_node *)coro->task)[ idx_task ];
	/*call api*/
	x_printf(D, "sharer func =======1========= %d %p\n", idx_task, coro->C[ idx_task ].func);
	/*
	 * one bug:
	 * when stack_size of C.stack is too small, g_sharer_uthread[1] run will stack overflow, and affect g_sharer_uthread[0] data change.
	 */
	if ( p_task->func == NULL ){
		x_printf(S, "TASK is NULL function!\n");
		return NULL;
	}
	int err = p_task->func( coro->data, p_task, idx_task );
	x_printf(D, "sharer func =======2========= %d %p\n", idx_task, coro->C[ idx_task ].func);

	if ( true == p_task->last ){
		x_printf(D, "task done over!\n");
		/*
		if (p_task->data != NULL){
			switch (p_task->origin){
				case BIT8_TASK_ORIGIN_MSMQ:
					free(p_task->data);
					break;
				case BIT8_TASK_ORIGIN_TIME:
					free(p_task->data);//TODO
					break;
				default:
					break;
			}
			p_task->data = NULL;
		}
		*/
	}
	return NULL;
}
#endif

#if defined(OPEN_LINE) || defined(OPEN_EVUV)
/*
 * 函 数:sniff_task_handle
 * 功 能:调用任务的回调处理函数　
 * 参 数:data指向线程的数据结构，addr指向任务
 * 返回值:
 * 说 明:
 */
static void sniff_task_handle(void *data, void *addr)
{
	struct sniff_task_node *p_task = addr;

	if ( p_task->func == NULL ){
		x_printf(S, "TASK is NULL function!\n");
		return;
	}
	/*call api*/
	int err = p_task->func( data, addr );
	
	if ( true == p_task->last ){
		x_printf(D, "task done over!\n");
		/*
		if (p_task->data != NULL){
			switch (p_task->origin){
				case BIT8_TASK_ORIGIN_MSMQ:
					free(p_task->data);
					break;
				case BIT8_TASK_ORIGIN_TIME:
					free(p_task->data);//TODO
					break;
				default:
					break;
			}
			p_task->data = NULL;
		}
		*/
	}
	return;
}
#endif

/*
 *函 数:start_parter
 *功 能:parter线程入口函数
 *参 数:
 * 返回值:
 * 说 明:
 */

static void *start_parter(void *arg)
{
	/* Any per-thread setup can happen here; thread_init() will block until
	 * all threads have finished initializing.
	 */
	struct safe_init_step *info = arg;
	//对info中的base进行加锁
	SAFE_PTHREAD_INIT_COME( info );

	PARTER_PTHREAD *p_parter = calloc( 1, sizeof(PARTER_PTHREAD) );
	assert( p_parter );
	((uintptr_t *)info->addr)[ info->step ] = (uintptr_t *)p_parter;

	struct sniff_bind_data *p_bind = (struct sniff_bind_data *)info->data;
	p_parter->data = p_bind->data;
	p_parter->batch = p_bind->batch;	//对应的porter线程的顺序号
	p_parter->genus = p_bind->genus;	//该parter线程组在porter线程中的顺序号

	p_parter->index = info->step;
	p_parter->thread_id = pthread_self();
	supex_task_init(&p_parter->tlist, sizeof(struct sniff_task_node), MAX_SNIFF_QUEUE_NUMBER);
	p_parter->glist = &g_supex_task_list;

	//对info中的base count加１,释放条件变量，释放锁
	SAFE_PTHREAD_INIT_OVER( info );
#ifdef OPEN_CORO
	p_parter->coro.num = G_SHARER_COUNTS_TIMES;
	p_parter->coro.tsz = sizeof(struct sniff_task_node);
	p_parter->coro.data = p_parter;
	p_parter->coro.task_lookup = g_sniff_settings.conf->task_lookup;
	p_parter->coro.task_handle = sniff_task_handle;

	supex_coro_loop(&p_parter->coro);
#endif
#ifdef OPEN_LINE
	p_parter->line.tsz = sizeof(struct sniff_task_node);
	p_parter->line.data = p_parter;
	p_parter->line.task_lookup = g_sniff_settings.conf->task_lookup;
	p_parter->line.task_handle = sniff_task_handle;

	//从用数组实现的无锁队列中取出一个任务进行处理
	supex_line_loop(&p_parter->line);
#endif
#ifdef OPEN_EVUV
	p_parter->evuv.tsz = sizeof(struct sniff_task_node);
	p_parter->evuv.data = p_parter;
	p_parter->evuv.task_lookup = g_sniff_settings.conf->task_lookup;
	p_parter->evuv.task_handle = sniff_task_handle;

	supex_evuv_loop(&p_parter->evuv);
#endif
	return NULL;
}

int sniff_mount(struct sniff_cfg_list *conf)
{
	SAFE_ONCE_INIT_COME( &g_sniff_mount_mark );
	
	cfg_check(conf);
	g_sniff_settings.conf = conf;

	supex_task_init(&g_supex_task_list, sizeof(struct sniff_task_node), MAX_SNIFF_SHARE_QUEUE_NUMBER);
	
	SAFE_ONCE_INIT_OVER( &g_sniff_mount_mark );
	return 0;
}


/*
 * 函 数:sniff_start
 * 功 能:启动porter附加的parter线程
 * 参 数:data指向porter,batch 对应的porter线程的顺序号,genus parter线程的生成号
 */
PARTER_PTHREAD *sniff_start(void *data, int batch, int genus)
{
	/*init data*/
	first_init();
	/*init proj*/
	if (g_sniff_settings.conf->entry_init){
		g_sniff_settings.conf->entry_init();
	}

	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|==start parter==|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	uintptr_t *addr_list[ G_PARTER_COUNTS ];
	struct sniff_bind_data bind_data = {
		.batch = batch,
		.genus = genus,
		.data = data
	};
	safe_start_pthread((void *)start_parter, G_PARTER_COUNTS, addr_list, &bind_data);

	//PARTER_PTHREAD 组成一个环形链表
	PARTER_PTHREAD *p_head = addr_list[0];
	PARTER_PTHREAD *p_temp = p_head;
	int i = 0;
	for (i = 1; i < G_PARTER_COUNTS; i++, p_temp = p_temp->next){
		p_temp->next = addr_list[i];
	}
	p_temp->next = p_head;
	/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>|================|<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	struct sniff_task_node task_init = {
		.sfd	= 0,
		.type	= BIT8_TASK_TYPE_WHOLE,
		.origin	= BIT8_TASK_ORIGIN_INIT,
		.func	= g_sniff_settings.conf->vmsys_init,
		.size	= 0
	};
	sniff_all_task_hit( p_head, &task_init );

	return p_head;
}
