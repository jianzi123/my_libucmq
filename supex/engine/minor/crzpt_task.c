#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "crzpt_task.h"

static struct share_task_view g_share_task_view[ MAX_CRZPT_QUEUE_NUMBER ] = {};

int crzpt_task_rgst(char origin, char type, short workers, short taskers)
{
	int  i = 0;
	int id = -1;
	int ok = false;
	switch ( origin ) {
		case BIT8_TASK_ORIGIN_INIT:
		case BIT8_TASK_ORIGIN_MSMQ:
		case BIT8_TASK_ORIGIN_TIME:
			for (i = 0; i < MAX_CRZPT_QUEUE_NUMBER; ++i) {
				if( __sync_bool_compare_and_swap(&g_share_task_view[ i ].cntl, TASK_ID_UNUSE, TASK_ID_INUSE) ){

					ok = true;
					id = i;
					break;
				}
			}
			break;
		default:
			return -1;
	}
	if ( !ok ) {
		return -1;
	}

	g_share_task_view[ id ].worker_finish = 0;
	g_share_task_view[ id ].tasker_finish = 0;
			
	switch ( type ) {
		case BIT8_TASK_TYPE_ALONE:
			g_share_task_view[ id ].worker_affect = 1;
			g_share_task_view[ id ].tasker_affect = 1;
			break;
		case BIT8_TASK_TYPE_WHOLE:
			g_share_task_view[ id ].worker_affect = workers;
			g_share_task_view[ id ].tasker_affect = taskers;
			break;
		default:
			assert( __sync_bool_compare_and_swap(&g_share_task_view[ id ].cntl, TASK_ID_INUSE, TASK_ID_UNUSE) );
			return -1;
	}
	return id;
}

void crzpt_task_come(int *store, int id)//FIXME
{
	int idx = __sync_add_and_fetch(&g_share_task_view[ id ].tasker_finish, 1);
	*store = idx;
	return;
}

int crzpt_task_last(int *store, int id)//FIXME
{
	int max = g_share_task_view[ id ].tasker_affect;
	return (*store == max)? true : false;
}

int crzpt_task_over(int id)
{
	assert( g_share_task_view[ id ].cntl == TASK_ID_INUSE );
	int max = g_share_task_view[ id ].worker_affect;/*must add before get idx or before unlock*/
	int idx = __sync_fetch_and_add(&g_share_task_view[ id ].worker_finish, 1);
	if ( max > 0 && idx == (max - 1) ){
		g_share_task_view[ id ].worker_finish = 0;
		g_share_task_view[ id ].worker_affect = 0;

		g_share_task_view[ id ].tasker_finish = 0;
		g_share_task_view[ id ].tasker_affect = 0;
		assert( __sync_bool_compare_and_swap(&g_share_task_view[ id ].cntl, TASK_ID_INUSE, TASK_ID_UNUSE) );
		return 0;
	}
	return (max - idx);
}
