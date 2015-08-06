#pragma once

#define FREE_LOCK_QUEUE

#define BIT8_TASK_TYPE_ALONE		'L'
#define BIT8_TASK_TYPE_WHOLE		'H'

#define BIT8_TASK_ORIGIN_HTTP		'p'
#define BIT8_TASK_ORIGIN_REDIS		'r'
#define BIT8_TASK_ORIGIN_MSMQ		'q'
#define BIT8_TASK_ORIGIN_TIME		'e'
#define BIT8_TASK_ORIGIN_INIT		'i'

#define TASK_ID_UNUSE			0
#define TASK_ID_INUSE			1

#define TASK_IS_BEGIN			0
#define TASK_IS_FINISH			1


struct share_task_view {
	short worker_affect;
	short worker_finish;

	short tasker_affect;
	short tasker_finish;

	short cntl;	/*this id if register*/
};
