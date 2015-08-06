#pragma once
#include "coro.h"
#include "utils.h"
#include "def_task.h"


#define MAX_SNIFF_QUEUE_NUMBER			(MAX_CONNECT * 2)//must > 2
#define MAX_SNIFF_SHARE_QUEUE_NUMBER		(MAX_SNIFF_DATA_SIZE * 16)
#define MAX_SNIFF_DATA_SIZE			6144//(MAX_REQ_SIZE)

struct sniff_task_node {
	int sfd;

	char type;
	char origin;
	SUPEX_TASK_CALLBACK func;
	bool last;						//是否为最后一个，用在sniff_all_task_hit中，代表这是最后一个parter线程处理该任务
	unsigned int shift;
	time_t stamp;
	pthread_t thread_id;			/* unique ID of this thread */
	int size;
	char data[ MAX_SNIFF_DATA_SIZE ];
};
