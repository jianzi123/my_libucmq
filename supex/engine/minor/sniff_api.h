#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "list.h"
#include "utils.h"
#include "supex.h"
#include "sniff_cfg.h"
#include "sniff_task.h"



struct sniff_settings {
	struct sniff_cfg_list *conf;
};

typedef struct sniff_parter_pthread {
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
	int genus;		//parter 线程组对应的porter的顺序号
	int index;		//parter 线程在线程组中的顺序号
	pthread_t thread_id;			/* unique ID of this thread */
	int thave;				/*can't use unsigned int*/
	struct supex_task_list tlist;	//用数组实现的无锁队列
	struct supex_task_list *glist;
	struct sniff_parter_pthread *next;
} PARTER_PTHREAD;

int sniff_mount(struct sniff_cfg_list *conf);

PARTER_PTHREAD *sniff_start(void *data, int batch, int genus);


typedef void (*SNIFF_VMS_ERR)( void **base );
#ifdef OPEN_CORO
typedef int (*SNIFF_VMS_FCB)( void **base, int last, struct sniff_task_node *task, long S );

int sniff_for_alone_vm( void *user, void *task, int step, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err );

int sniff_for_batch_vm( void *user, void *task, int step, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err );
#endif
#if defined(OPEN_LINE) || defined(OPEN_EVUV)
typedef int (*SNIFF_VMS_FCB)( void **base, int last, struct sniff_task_node *task );

int sniff_for_alone_vm( void *user, void *task, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err );
#endif

void sniff_all_task_hit( PARTER_PTHREAD *head, struct sniff_task_node *task );

void sniff_one_task_hit( PARTER_PTHREAD *head, struct sniff_task_node *task );
