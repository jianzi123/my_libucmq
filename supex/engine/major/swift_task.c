#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "swift_task.h"

static struct share_task_view g_share_task_view[ MAX_SWIFT_QUEUE_NUMBER ] = {};

/*
 * 函 数:swift_task_rgst
 * 功 能:注册任务
 * 参 数:origin 任务来源，type 任务的类型, workers 任务要多少个worker处理
 * 返回值:返回任务在g_share_task_view中的下标
 * 说 明:
 */
int swift_task_rgst(char origin, char type, short workers, short taskers, int mark)
{
	int  i = 0;
	int id = -1;
	int ok = false;
	switch ( origin ) {
		//对于BIT8_TASK_ORIGIN_HTTP和BIT8_TASK_ORIGIN_REDIS来说mark 是sfd,
		case BIT8_TASK_ORIGIN_HTTP:
		case BIT8_TASK_ORIGIN_REDIS:
			assert( __sync_bool_compare_and_swap(&g_share_task_view[ mark ].cntl, TASK_ID_UNUSE, TASK_ID_INUSE) );

			ok = true;
			id = mark;
			break;
		case BIT8_TASK_ORIGIN_INIT:
		case BIT8_TASK_ORIGIN_MSMQ:
		case BIT8_TASK_ORIGIN_TIME:
			for (i = MAX_SWIFT_HTTP_NUMBER; i < MAX_SWIFT_QUEUE_NUMBER; ++i) {
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

void swift_task_come(int *store, int id)//FIXME
{
	int idx = __sync_add_and_fetch(&g_share_task_view[ id ].tasker_finish, 1);
	*store = idx;
	return;
}

int swift_task_last(int *store, int id)//FIXME
{
	int max = g_share_task_view[ id ].tasker_affect;
	return (*store == max)? true : false;
}

/*
 *函 数:swift_task_over
 *功 能:查看g_share_task_view下标为id对应的任务是否完成了(通过worker_affect和worker_finish进行比较得知)
 *参 数:
 *返回值:执行完返回０,未执行完返回大于０的数，即未完成的次数
 *说 明:
 */
int swift_task_over(int id)
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
