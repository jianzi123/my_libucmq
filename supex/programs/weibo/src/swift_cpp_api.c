#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "errors.h"
#include "swift_api.h"
#include "sniff_api.h"
#include "swift_cpp_api.h"
#include "apply_entry.h"
#include "cJSON.h"
#include "sniff_evuv_cpp_api.h"

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
	char bizid[50] = {};
	char buf[255] ={};
	char error_string[100] = {};
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)W;
	struct swift_task_node *swift_task = &p_porter->task;
	struct data_node *p_node = get_pool_addr( swift_task->sfd );
	char *p_buf = p_node->recv.buf_addr;
	struct redis_status *p_rst = &p_node->redis_info.rs;

	struct sniff_task_node sniff_task = {};

	if ( !strncmp(p_buf + p_rst->key_offset[0], "personal", MAX(p_rst->klen_array[0], 8)) ){
		int ok = entry_personal_weibo( p_porter->loop, p_buf + p_rst->val_offset[0], p_rst->vlen_array[0],bizid );
		if ( ok !=API_OK){
			sprintf(error_string,"+%s\r\n",handle_error(ok));
			cache_add( &p_node->send, error_string, strlen(error_string) );
			return NULL;
		}
		sprintf(buf,"+%s\r\n",bizid);
		cache_add( &p_node->send, buf, strlen(buf) );
		return 	0;
	}
	else if ( !strncmp(p_buf + p_rst->key_offset[0], "group", MAX(p_rst->klen_array[0], 5)) ){
		/*create bizid*/
		strcat(bizid, "a2");
		create_uuid(bizid);
		sprintf(buf,",\"bizid\":\"%s\"}",bizid);
		strcpy(strlen(p_buf + p_rst->val_offset[0])-3+p_buf + p_rst->val_offset[0],buf);

	}else if ( !strncmp(p_buf + p_rst->key_offset[0], "city", MAX(p_rst->klen_array[0], 4)) ){
		/*create bizid*/
		strcat(bizid, "a6");
		create_uuid(bizid);
		sprintf(buf,",\"bizid\":\"%s\"}",bizid);
		strcpy(strlen(p_buf + p_rst->val_offset[0])-3+p_buf + p_rst->val_offset[0],buf);
	}else{
		cache_add( &p_node->send, OPT_MULTI_BULK_FALSE, strlen(OPT_MULTI_BULK_FALSE) );
		return NULL;
	}

	sniff_task.sfd = swift_task->sfd;
	sniff_task.type = swift_task->type;
	sniff_task.origin = swift_task->origin;
	sniff_task.func = sniff_vms_call;//fix to use xxxx;
	sniff_task.last = false;//FIXME
	sniff_task.stamp = time(NULL);
	assert(p_rst->vlen_array[0] < MAX_SNIFF_DATA_SIZE);
	sniff_task.size = MIN(p_rst->vlen_array[0]+strlen(buf), MAX_SNIFF_DATA_SIZE - 1);
	memcpy(sniff_task.data, p_buf + p_rst->val_offset[0], sniff_task.size);

#if 0
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)p_porter->mount;
#else
	PARTER_PTHREAD *p_parter = get_idle_thread( (PARTER_PTHREAD *)p_porter->mount );
#endif
	sniff_task.thread_id = p_parter->thread_id;
	sniff_one_task_hit( p_parter, &sniff_task );
	supex_evuv_wake(&p_parter->evuv);
	p_porter->mount = p_parter->next;
	memset(buf,0,sizeof(buf));
	sprintf(buf,"+%s\r\n",bizid);
	cache_add( &p_node->send, buf, strlen(buf) );
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
