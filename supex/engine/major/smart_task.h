#pragma once
#include "coro.h"
#include "utils.h"
#include "def_task.h"


#define MAX_SMART_HTTP_NUMBER			MAX_LIMIT_FD
#define MAX_SMART_MSMQ_NUMBER			5
#define MAX_SMART_PLAN_NUMBER			1
#define MAX_SMART_QUEUE_NUMBER			(MAX_SMART_HTTP_NUMBER + MAX_SMART_MSMQ_NUMBER + MAX_SMART_PLAN_NUMBER)


struct smart_task_node {
	int id;
	int sfd;

	char type;
	char origin;
	SUPEX_TASK_CALLBACK func;
	int index;
	char *deal;
	void *data;
};

/*---------------------------------------------------------*/

int smart_task_rgst(char origin, char type, short workers, short taskers, int mark);

int smart_task_over(int id);


void smart_task_come(int *store, int id);

int smart_task_last(int *store, int id);
