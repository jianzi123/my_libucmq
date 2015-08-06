#pragma once
#include "coro.h"
#include "utils.h"
#include "def_task.h"


#define MAX_CRZPT_MSMQ_NUMBER			MAX_CONNECT
#define MAX_CRZPT_PLAN_NUMBER			(1 + 9999)
#define MAX_CRZPT_QUEUE_NUMBER			(MAX_CRZPT_MSMQ_NUMBER + MAX_CRZPT_PLAN_NUMBER)

enum {
	CRZPT_MSMQ_TYPE_PUSH = 1,	//for lua VM
	CRZPT_MSMQ_TYPE_PULL,		//for lua VM

	CRZPT_MSMQ_TYPE_INSERT,		//for task list
	CRZPT_MSMQ_TYPE_UPDATE,		//for task list
	CRZPT_MSMQ_TYPE_SELECT,		//for task list
};

struct crzpt_task_node {
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

int crzpt_task_rgst(char origin, char type, short workers, short taskers);

int crzpt_task_over(int id);


void crzpt_task_come(int *store, int id);

int crzpt_task_last(int *store, int id);
