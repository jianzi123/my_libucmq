#include <string.h>

#include "smart_coro_cpp_api.h"
#include "utils.h"
#include "smart_api.h"
#include "entry.h"


int test_cfunc( void *user, void *task, int step )
{
	WORKER_PTHREAD *p_worker = (WORKER_PTHREAD *)user;
	struct smart_task_node *p_task = (struct smart_task_node *)task;

	printf("run done.....\n");
	smart_task_come( &p_task->index, p_task->id );

	int sfd = p_task->sfd;

	//smart_task_last( &p_task->index, p_task->id );
	
	struct data_node *p_node = get_pool_addr( sfd );
	cache_add( &p_node->send, OPT_OK, strlen(OPT_OK) );
	return 0;
}
