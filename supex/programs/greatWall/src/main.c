#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "mq_api.h"

#include "swift_api.h"
#include "swift_cpp_api.h"
#include "load_swift_cfg.h"

#include "sniff_api.h"
#include "load_sniff_cfg.h"
#include "apply_def.h"
#include "dams_cfg.h"

#ifdef OPEN_CORO
#include "sniff_coro_lua_api.h"
#else
#include "sniff_line_cpp_api.h"
#endif


struct swift_cfg_list g_swift_cfg_list = {};
struct sniff_cfg_list g_sniff_cfg_list = {};
struct dams_cfg_file g_dams_cfg_file = {};
/*
 *函 数:swift_pthrd_init
 *功 能:porter线程启动时会调用swift_pthrd_init用来开启parter线程组
 */
static void swift_pthrd_init(void *user)
{
	PORTER_PTHREAD *p_porter = user;
	struct mount_info *link = calloc(LIMIT_CHANNEL_KIND, sizeof(struct mount_info));
	int i = 0;
	for (i=0; i < LIMIT_CHANNEL_KIND; i++) {
		//开启parter 线程组
		link[i].list = sniff_start( p_porter, p_porter->index, i );
		link[i].next = &link[i+1];
	}
	link[ LIMIT_CHANNEL_KIND - 1 ].next = NULL;
	p_porter->mount = link;
}

#ifdef STORE_USE_QUEUE
static bool sniff_task_report(void *user, void *task)
{
	return supex_task_push(&((PARTER_PTHREAD *)user)->tlist, task);
}
/*
 * 函 数:sniff_task_lookup
 * 功 能:从用户的用数组实现的无锁队列tlist中取出一个任务，保存到task中
 */
static bool sniff_task_lookup(void *user, void *task)
{
    //从用户的用数组实现的无锁队列tlist中取出一个任务，保存到task中
	return supex_task_pull(&((PARTER_PTHREAD *)user)->tlist, task);
}
#endif

#ifdef STORE_USE_UCMQ
static bool sniff_task_report(void *user, void *task)
{
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)user;
	char temp[32] = {};
	sprintf(temp, "%d_%d_%d", p_parter->batch, p_parter->genus, p_parter->index);
	bool ok = mq_store_put(temp, task, sizeof(struct sniff_task_node));
	if (!ok) {
		x_printf(E, "push failed!\n");
	}else{
		x_printf(D, "push ok!\n");
		//struct sniff_task_node *p_task = task;
		//printf("function %p\n", p_task->func);
	}
	return ok;
}
static bool sniff_task_lookup(void *user, void *task)
{
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)user;
	char temp[32] = {};
	sprintf(temp, "%d_%d_%d", p_parter->batch, p_parter->genus, p_parter->index);
	bool ok = mq_store_get(temp, task, sizeof(struct sniff_task_node));
	if (!ok) {
		//x_printf(D, "pull NULL!\n");
	}else{
		x_printf(D, "pull ok!\n");
		//struct sniff_task_node *p_task = task;
		//printf("function %p\n", p_task->func);
	}
	return ok;
}
#endif
#ifdef STORE_USE_UCMQ_AND_QUEUE
#define MAX_SNIFF_TEMP_QUEUE_NUMBER		2//must >= 2
enum {
	MARK_USE_QUEUE = 0,
	MARK_USE_UCMQ,
};
struct queue_stat_info {
	int mark_report;
	unsigned int shift_report;
	int mark_lookup;
	unsigned int shift_lookup;
	int step_lookup;
	struct sniff_task_node temp;
	struct supex_task_list swap;
};

static struct queue_stat_info *g_queue_stat_list = NULL;

static bool sniff_task_report(void *user, void *task)
{
	bool ok = false;
	int new_mark = 0;
	unsigned int new_shift = 0;
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)user;
	struct sniff_task_node *p_task = (struct sniff_task_node *)task;
	struct queue_stat_info *p_stat = &g_queue_stat_list[ p_parter->batch * LIMIT_CHANNEL_KIND * g_sniff_cfg_list.file_info.parter_counts + p_parter->genus * g_sniff_cfg_list.file_info.parter_counts +  p_parter->index ];

	do {
		//---->use queue
		new_mark = MARK_USE_QUEUE;
		if (p_stat->mark_report == MARK_USE_QUEUE){
			p_task->shift = p_stat->shift_report;
		}else{
			p_task->shift = p_stat->shift_report + 1;
		}
		new_shift = p_task->shift;
		ok = supex_task_push(&p_parter->tlist, p_task);
		if (ok){
			x_printf(D, "push queue ok!\n");
			p_stat->mark_report = new_mark;
			p_stat->shift_report = new_shift;
#if 1
			__sync_add_and_fetch(&p_parter->thave, 1);
#endif
			break;
		}else{
			x_printf(W, "push queue failed!\n");
		}

		/*******************/
		char temp[32] = {};
		sprintf(temp, "%d_%d_%d", p_parter->batch, p_parter->genus, p_parter->index);
		/*******************/

		//---->use ucmq
		new_mark = MARK_USE_UCMQ;
		if (p_stat->mark_report == MARK_USE_UCMQ){
			p_task->shift = p_stat->shift_report;
		}else{
			p_task->shift = p_stat->shift_report + 1;
		}
		new_shift = p_task->shift;
		ok = mq_store_put(temp, task, sizeof(struct sniff_task_node));
		if (ok) {
			x_printf(D, "push ucmq ok!\n");
			p_stat->mark_report = new_mark;
			p_stat->shift_report = new_shift;
#if 1
			__sync_add_and_fetch(&p_parter->thave, 1);
#endif
			break;
		}else{
			x_printf(E, "push ucmq failed!\n");
		}
	}while(0);

	return ok;
}

static bool sniff_task_lookup(void *user, void *task)
{
	char temp[32] = {};
	bool ok = false;
#if 1
	int have = 0;
#endif
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)user;
	struct sniff_task_node *p_task = (struct sniff_task_node *)task;
	struct queue_stat_info *p_stat = &g_queue_stat_list[ p_parter->batch * LIMIT_CHANNEL_KIND * g_sniff_cfg_list.file_info.parter_counts + p_parter->genus * g_sniff_cfg_list.file_info.parter_counts +  p_parter->index ];
#if 1
	have = p_parter->thave;
	if (have <= 0 && p_stat->step_lookup == 2){
		return supex_task_pull(p_parter->glist, p_task);
	}
#endif
	switch(p_stat->step_lookup){
		case 0:
			/*init vms step*/
			ok = supex_task_pull(&p_parter->tlist, p_task);
			if (ok){
#if 1
				__sync_sub_and_fetch(&p_parter->thave, 1);
#endif
				p_stat->step_lookup ++;
			}
			break;
		case 1:
			/*done old step*/
			/*******************/
			sprintf(temp, "%d_%d_%d", p_parter->batch, p_parter->genus, p_parter->index);
			/*******************/
			ok = mq_store_get(temp, task, sizeof(struct sniff_task_node));
			if (ok){
				if (p_task->shift == 1){
					supex_task_push(&p_stat->swap, p_task);//push l
					p_stat->step_lookup ++;
				}else{
					//do nothing, is old task
				}
			}else{
				p_stat->step_lookup ++;
			}
			break;
		case 2:
			/*done new step*/
			if (p_stat->mark_lookup == MARK_USE_QUEUE){
				ok = supex_task_pull(&p_parter->tlist, p_task);
				if (ok){
					if (p_task->shift == p_stat->shift_lookup){
						//do nothing, is next task
#if 1
						__sync_sub_and_fetch(&p_parter->thave, 1);
#endif
					}else{
						/*not first push then pop,it will get the push task in one loop.*/
						memcpy(&p_stat->temp, p_task, sizeof(struct sniff_task_node));
						ok = supex_task_pull(&p_stat->swap, p_task);//pull l
#if 1
						if (ok){
							__sync_sub_and_fetch(&p_parter->thave, 1);
						}
#endif
						supex_task_push(&p_stat->swap, &p_stat->temp);//push l
						p_stat->shift_lookup ++;
						p_stat->mark_lookup = MARK_USE_UCMQ;
					}
				}else{
#if 0
					/*maybe no task come,so add idle task come*/
					//do nothing, no task
#else
					if (have > 0){
						ok = supex_task_pull(&p_stat->swap, p_task);//pull l
						if (ok){
							__sync_sub_and_fetch(&p_parter->thave, 1);
						}
						p_stat->shift_lookup ++;
						p_stat->mark_lookup = MARK_USE_UCMQ;
					}else{
						//do nothing, no task
					}
#endif
				}
			}else{
				/*******************/
				sprintf(temp, "%d_%d_%d", p_parter->batch, p_parter->genus, p_parter->index);
				/*******************/
				ok = mq_store_get(temp, task, sizeof(struct sniff_task_node));
				if (ok){
					if (p_task->shift == p_stat->shift_lookup){
						//do nothing, is next task
#if 1
						__sync_sub_and_fetch(&p_parter->thave, 1);
#endif
					}else{
						memcpy(&p_stat->temp, p_task, sizeof(struct sniff_task_node));
						ok = supex_task_pull(&p_stat->swap, p_task);//pull l
#if 1
						if (ok){
							__sync_sub_and_fetch(&p_parter->thave, 1);
						}
#endif
						supex_task_push(&p_stat->swap, &p_stat->temp);//push l
						p_stat->shift_lookup ++;
						p_stat->mark_lookup = MARK_USE_QUEUE;
					}
				}else{
#if 0
					//do nothing, no task
#else
					if (have > 0){
						ok = supex_task_pull(&p_stat->swap, p_task);//pull l
						if (ok){
							__sync_sub_and_fetch(&p_parter->thave, 1);
						}
						p_stat->shift_lookup ++;
						p_stat->mark_lookup = MARK_USE_QUEUE;
					}else{
						//do nothing, no task
					}
#endif
				}
			}
			break;
		default:
			break;
	}
	return ok;
}
#endif



void swift_entry_init(void)
{
#if defined(STORE_USE_UCMQ) || defined(STORE_USE_UCMQ_AND_QUEUE)
	bool ok = mq_store_init("./mq_data/logs", "./mq_data/data");
	if (!ok) {
		x_perror("mq_store_init");
		exit(EXIT_FAILURE);
	}
#ifdef STORE_USE_UCMQ_AND_QUEUE
	int all = g_swift_cfg_list.file_info.porter_counts * g_sniff_cfg_list.file_info.parter_counts * LIMIT_CHANNEL_KIND;
	g_queue_stat_list = calloc(all, sizeof(struct queue_stat_info));
	assert(g_queue_stat_list);
	struct queue_stat_info *p_stat = NULL;
	while(all--){
		p_stat = &g_queue_stat_list[ all ];
		supex_task_init(&p_stat->swap, sizeof(struct sniff_task_node), MAX_SNIFF_TEMP_QUEUE_NUMBER);
	}
#endif
#endif
}

int main(int argc, char** argv)
{
	//---> init swift
	load_swift_cfg_argv(&g_swift_cfg_list.argv_info, argc, argv);

	load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	g_swift_cfg_list.func_info[ LPUSHX_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[ LPUSHX_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_call;
	g_swift_cfg_list.func_info[ RPUSHX_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[ RPUSHX_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_call;

	g_swift_cfg_list.entry_init = swift_entry_init;
	g_swift_cfg_list.pthrd_init = swift_pthrd_init;

	g_swift_cfg_list.vmsys_init = swift_vms_init;
#ifdef STORE_USE_UCMQ_AND_QUEUE
#if 0
	g_swift_cfg_list.vmsys_idle = swift_vms_idle;
#else
	/*have bug when tasks pile up*/
#endif
#endif

	swift_mount(&g_swift_cfg_list);


	//---> init route
	read_dams_cfg(&g_dams_cfg_file, g_swift_cfg_list.argv_info.conf_name);
	//---> init sniff
	memcpy( g_sniff_cfg_list.argv_info.conf_name, 
			g_swift_cfg_list.argv_info.conf_name, MAX_FILE_NAME_SIZE );
	memcpy( g_sniff_cfg_list.argv_info.serv_name,
			g_swift_cfg_list.argv_info.serv_name, MAX_FILE_NAME_SIZE );
	memcpy( g_sniff_cfg_list.argv_info.msmq_name,
			g_swift_cfg_list.argv_info.msmq_name, MAX_FILE_NAME_SIZE );

	load_sniff_cfg_file(&g_sniff_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);
/*
	g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].func = (TASK_CALLBACK)sniff_vms_call;
*/
	g_sniff_cfg_list.task_lookup = sniff_task_lookup;
	g_sniff_cfg_list.task_report = sniff_task_report;
	
	g_sniff_cfg_list.vmsys_init = sniff_vms_init;


	sniff_mount(&g_sniff_cfg_list);

	swift_start();
	return 0;
}
