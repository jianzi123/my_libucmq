#include <string.h>
#include <stdlib.h>

#include "smart_line_cpp_api.h"
#include "utils.h"
#include "smart_api.h"
#include "entry.h"


int api_hmget(void *user, void *task)
{
	WORKER_PTHREAD *p_worker = (WORKER_PTHREAD *)user;
	struct smart_task_node *p_task = (struct smart_task_node *)task;
	struct data_node *p_node = get_pool_addr( p_task->sfd );

	char *p_buf = p_node->recv.buf_addr;
	struct redis_status *p_rst = &p_node->redis_info.rs;

	//printf("%d %s\n", p_rst->flen_array[0], p_buf + p_rst->fld_offset[0]);
	if ( !strncmp(p_buf + p_rst->fld_offset[0], "ERBR", MAX(p_rst->flen_array[0], 4)) ){
		return entry_cmd_erbr( p_node,
				strtoull(p_buf + p_rst->key_offset[0], NULL, 10) );
	}
	if ( !strncmp(p_buf + p_rst->fld_offset[0], "IRBR", MAX(p_rst->flen_array[0], 4)) ){
		return entry_cmd_irbr( p_node,
				strtoull(p_buf + p_rst->key_offset[0], NULL, 10) );
	}

	if ( !strncmp(p_buf + p_rst->fld_offset[0], "EROF", MAX(p_rst->flen_array[0], 4)) ){
		return entry_cmd_erof( p_node,
				strtoull(p_buf + p_rst->key_offset[0], NULL, 10) );
	}
	if ( !strncmp(p_buf + p_rst->fld_offset[0], "IROF", MAX(p_rst->flen_array[0], 4)) ){
		return entry_cmd_irof( p_node,
				strtoull(p_buf + p_rst->key_offset[0], NULL, 10) );
	}

	if ( !strncmp(p_buf + p_rst->fld_offset[0], "ENBR", MAX(p_rst->flen_array[0], 4)) ){
		return entry_cmd_enbr( p_node,
				strtoull(p_buf + p_rst->key_offset[0], NULL, 10) );
	}
	if ( !strncmp(p_buf + p_rst->fld_offset[0], "FNBR", MAX(p_rst->flen_array[0], 4)) ){
		return entry_cmd_fnbr( p_node,
				strtoull(p_buf + p_rst->key_offset[0], NULL, 10) );
	}

	cache_add( &p_node->send, OPT_MULTI_BULK_FALSE, strlen(OPT_MULTI_BULK_FALSE) );
	return 0;
}

int api_hgetall(void *user, void *task)
{
	WORKER_PTHREAD *p_worker = (WORKER_PTHREAD *)user;
	struct smart_task_node *p_task = (struct smart_task_node *)task;
	struct data_node *p_node = get_pool_addr( p_task->sfd );

	char *p_buf = p_node->recv.buf_addr;
	struct redis_status *p_status = &p_node->redis_info.rs;

	if ( !strncmp(p_buf + p_status->key_offset[0], "BEELINE", 7) ){
		return entry_cmd_rlbr( p_node, 0 );
	}
	/*
	if ( !strncmp(p_buf + p_status->key_offset[0], "SCALLOP", 7) ){
		return entry_cmd_rlbr( p_node, 0 );
	}
	*/

	cache_add( &p_node->send, OPT_OK, strlen(OPT_OK) );
	return 0;
}
