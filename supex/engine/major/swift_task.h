#pragma once
#include "coro.h"
#include "utils.h"
#include "def_task.h"


#define MAX_SWIFT_HTTP_NUMBER			MAX_LIMIT_FD
#define MAX_SWIFT_MSMQ_NUMBER			5
#define MAX_SWIFT_PLAN_NUMBER			1
#define MAX_SWIFT_QUEUE_NUMBER			(MAX_SWIFT_HTTP_NUMBER + MAX_SWIFT_MSMQ_NUMBER + MAX_SWIFT_PLAN_NUMBER)



struct swift_task_node {
	int id;
	int sfd;

	char type;		//任务类型，有单点任务和全局任务两种
	char origin;	//数据来源，有http,redis,msmq,time和init五种
	TASK_CALLBACK func;	//任务的回调函数
	int index;
	char *deal;
	void *data;
};

/*---------------------------------------------------------*/

int swift_task_rgst(char origin, char type, short workers, short taskers, int mark);

int swift_task_over(int id);


void swift_task_come(int *store, int id);

int swift_task_last(int *store, int id);

