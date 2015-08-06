#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "swift_api.h"
#include "sniff_api.h"
#include "swift_cpp_api.h"

#ifdef OPEN_CORO
#include "sniff_coro_lua_api.h"
#else
#include "sniff_line_lua_api.h"
#endif

extern struct sniff_cfg_list g_sniff_cfg_list;

int swift_vms_init( void *W )
{
	/*
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)W;
	struct swift_task_node *swift_task = &p_porter->task;

	struct sniff_task_node sniff_task = {};
	sniff_task.sfd = swift_task->sfd;
	sniff_task.type = swift_task->type;
	sniff_task.origin = swift_task->origin;
	sniff_task.func = g_sniff_cfg_list.vmsys_init;
	sniff_task.last = false;//FIXME
	sniff_task.size = 0;

	sniff_all_task_hit( (PARTER_PTHREAD *)p_porter->mount, &sniff_task );
	*/
	return 0;
}

PARTER_PTHREAD *get_idle_thread( PARTER_PTHREAD *p_list )
{
	PARTER_PTHREAD *p_idle = p_list;
	int idle = p_idle->thave;
	PARTER_PTHREAD *p_temp = p_list->next;

	while(p_temp != p_list){
		int temp = p_temp->thave;
		if (temp < idle){
			p_idle = p_temp;
			idle = temp;
		}
		p_temp = p_temp->next;
	}
	return p_idle;
}

int swift_vms_call( void *W )
{
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)W;
	struct swift_task_node *swift_task = &p_porter->task;

	struct sniff_task_node sniff_task = {};
	sniff_task.sfd = swift_task->sfd;
	sniff_task.type = swift_task->type;
	sniff_task.origin = swift_task->origin;
	sniff_task.func = sniff_vms_call;//fix to use xxxx;
	sniff_task.last = false;//FIXME
	sniff_task.stamp = time(NULL);
	get_cache_data(swift_task->sfd, sniff_task.data, &sniff_task.size);

#if 0
	sniff_task.thread_id = ((PARTER_PTHREAD *)p_porter->mount)->thread_id;
	sniff_one_task_hit( (PARTER_PTHREAD *)p_porter->mount, &sniff_task );
	p_porter->mount = ((PARTER_PTHREAD *)p_porter->mount)->next;
#else
	PARTER_PTHREAD *p_parter = get_idle_thread( (PARTER_PTHREAD *)p_porter->mount );
	sniff_task.thread_id = p_parter->thread_id;
	sniff_one_task_hit( p_parter, &sniff_task );
	p_porter->mount = p_parter->next;
#endif
	return 0;
}

int swift_vms_exec( void *W )
{
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)W;
	struct swift_task_node *swift_task = &p_porter->task;

	struct sniff_task_node sniff_task = {};
	sniff_task.sfd = swift_task->sfd;
	sniff_task.type = swift_task->type;
	sniff_task.origin = swift_task->origin;
	sniff_task.func = sniff_vms_exec;//fix to use xxxx;
	sniff_task.last = false;//FIXME
	sniff_task.stamp = time(NULL);
	get_cache_data(swift_task->sfd, sniff_task.data, &sniff_task.size);

#if 0
	sniff_task.thread_id = ((PARTER_PTHREAD *)p_porter->mount)->thread_id;
	sniff_one_task_hit( (PARTER_PTHREAD *)p_porter->mount, &sniff_task );
	p_porter->mount = ((PARTER_PTHREAD *)p_porter->mount)->next;
#else
	PARTER_PTHREAD *p_parter = get_idle_thread( (PARTER_PTHREAD *)p_porter->mount );
	sniff_task.thread_id = p_parter->thread_id;
	sniff_one_task_hit( p_parter, &sniff_task );
	p_porter->mount = p_parter->next;
#endif
	return 0;
}

#define BIT8_TASK_ORIGIN_IDLE		'd'
int swift_vms_idle( void *W )
{
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)W;

	struct sniff_task_node sniff_task = {};
	sniff_task.sfd = 0;
	sniff_task.type = BIT8_TASK_TYPE_WHOLE;
	sniff_task.origin = BIT8_TASK_ORIGIN_IDLE;
	sniff_task.func = sniff_vms_idle;//fix to use xxxx;
	sniff_task.last = false;//FIXME
	sniff_task.size = 0;

	sniff_all_task_hit( (PARTER_PTHREAD *)p_porter->mount, &sniff_task );
	return 0;
}
