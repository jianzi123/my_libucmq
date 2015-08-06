#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "smart_task.h"

static struct share_task_view g_share_task_view[ MAX_SMART_QUEUE_NUMBER ] = {};

int smart_task_rgst(char origin, char type, short workers, short taskers, int mark)
{
	int  i = 0;
	int id = -1;
	int ok = false;
	switch ( origin ) {
		case BIT8_TASK_ORIGIN_HTTP:
		case BIT8_TASK_ORIGIN_REDIS:
			assert( __sync_bool_compare_and_swap(&g_share_task_view[ mark ].cntl, TASK_ID_UNUSE, TASK_ID_INUSE) );

			ok = true;
			id = mark;
			break;
		case BIT8_TASK_ORIGIN_INIT:
		case BIT8_TASK_ORIGIN_MSMQ:
		case BIT8_TASK_ORIGIN_TIME:
			for (i = MAX_SMART_HTTP_NUMBER; i < MAX_SMART_QUEUE_NUMBER; ++i) {
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
/*
 * 函 数:smart_task_come
 * 功　能:返回id对应的任务被完成的次数
 * 参 数:store存储完成的次数，　id任务对应的下标
 * 返回值:
 * 修 改:添加注释  程少远2015/05/12
 */
void smart_task_come(int *store, int id)//FIXME
{
	int idx = __sync_add_and_fetch(&g_share_task_view[ id ].tasker_finish, 1);
	*store = idx;
	return;
}
/*
 * 函 数:smart_task_last
 * 功 能:判断任务是否已经被所有需要处理的工作线程处理完
 * 参 数:store 存储任务已经被处理的次数, id 该任务对应的索引
 * 返回值:处理完返回true, 没有处理完返回false
 * 修 改: 添加注释　程少远　2015/05/12　
 */
int smart_task_last(int *store, int id)//FIXME
{
	int max = g_share_task_view[ id ].tasker_affect;
	return (*store == max)? true : false;
}

int smart_task_over(int id)
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
