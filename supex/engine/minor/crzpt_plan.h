#pragma once

#include "utils.h"
#include "crzpt_task.h"


struct crzpt_plan_node {
	unsigned int pidx;
	enum {
		EXECUTE_DEATH = 0,
		EXECUTE_ALIVE = 1,
	} live;
	struct crzpt_plan_node *prev;
	struct crzpt_plan_node *next;
};

struct crzpt_time_node {
	int time;
	int status;
	struct crzpt_time_node *prev;
	struct crzpt_time_node *next;
	
	int nubs;
	struct crzpt_plan_node pl_head;/*not use*/
};

struct crzpt_time_list {
	int nubs;
	struct crzpt_time_node *done;
	struct crzpt_time_node tm_head;/*not use*/
};
/*---------------------------------------------------------*/
typedef void (*PLAY_FCB)( struct crzpt_plan_node *p_plan, struct crzpt_task_node *p_task );

void crzpt_set_start_time(int start_time);
int crzpt_get_next_time(int start_time);

void crzpt_plan_list_init(void);

struct crzpt_time_node *crzpt_get_plan_list(int end_time);

void *crzpt_cpp_make_plan(int time, short live, char *data);


//void free_app_tasks(int aidx);
void crzpt_list_for_each(struct crzpt_time_node *p_list, PLAY_FCB play_fcb, struct crzpt_task_node *p_task);

void crzpt_free_all_plan(void);
