#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "entry.h"
#include "net_cache.h"
#include "topo.h"
#include "topo_api.h"

/*
   if ( ret == X_MALLOC_FAILED ){
   clean_send_node(p_node);
   add_send_node(p_node, OPT_NO_MEMORY, strlen(OPT_NO_MEMORY));
   return X_MALLOC_FAILED;
   }
   */

void entry_init(void)
{
	topo_start( "topology_conf.json" );
#if 0
	char tmp [1024];
	int len;
	while ( 1 == 1) {
		//scanf("%s", tmp);
		memset(tmp, 0, 1024);
		fgets(tmp, 1024, stdin );
		len = strlen(tmp);
		if ( len == 6 && strncmp(tmp, "login", 5) == 0)
			break;
		//fprintf(stdout, "[tmp = %s]\n", tmp);
	}
#else
	char shell[1024] = {0};
	//char format[] = "unset kid; for pid in `pidof topology`; do if [ $pid -ne %d ]; then kid+=$pid" "; fi done; if [ $kid ]; then kill -9 $kid; fi";
	char format[] = "unset kid; for pid in `pidof topology`; do kid+=${pid//%d/}" "; done; if [ $kid ]; then kill -9 $kid; fi";
	sprintf(shell, format, getpid());
	system( shell );
#endif
}



static int get_number_len(uint64_t len)
{
	int i= 1;
	while ((len /= 10) >= 1){
		i++;
	}
	return i;
}

static int put_number_out(struct net_cache *p_cache, uint64_t nb)
{
	char tmp[32] = {0};
	sprintf(tmp, "%zu\r\n", nb);
	return cache_add( p_cache, tmp, strnlen(tmp, 32) );
}


static int entry_topo_main_call(struct data_node *p_node, uint64_t idx, TOPO_CALLBACK topo_cb, int max)
{
	int ok = false;
	struct net_cache *p_cache = &p_node->send;
	uint64_t slot[ max ];
	struct query_args info = {};
	info.idx = idx;
	info.buf = slot;
	info.peak = max;


	ok = topo_cb(&info);
	if (false == ok){
		cache_add( p_cache, "-", 1 );
		cache_add( p_cache, info.erro, strnlen(info.erro, MAX_ERROR_MESSAGE_SIZE) );
		cache_add( p_cache, "\r\n", 2 );
		return -1;
	}
	if (0 == info.size){
		cache_add( p_cache, OPT_MULTI_BULK_NULL, strlen(OPT_MULTI_BULK_NULL) );
		return 0;
	}
	// *2\r\n$5\r\nmykey\r\n$5\r\nmyval\r\n 
	int i = 0;
	cache_add( p_cache, "*", 1 );
	put_number_out(p_cache, info.size);
	for(; i < info.size; i++){
		cache_add( p_cache, "$", 1 );
		put_number_out( p_cache, get_number_len( info.buf[i] ) );
		put_number_out( p_cache, info.buf[i] );
	}
	return 0;
}


static int entry_topo_with_data_call(struct data_node *p_node, uint64_t idx, TOPO_CALLBACK topo_cb, int max)
{
	int ok = false;
	struct net_cache *p_cache = &p_node->send;
	uint64_t slot[ max ];
	char *p_buf = p_node->recv.buf_addr;
	struct redis_status *p_status = &p_node->redis_info.rs;
	struct query_args info = {};
	memset(&info, 0, sizeof(info));
	info.idx = idx;
	info.buf = slot;
	info.peak = max;
	strncpy(info.data, p_buf + p_status->key_offset[0], p_status->klen_array[0]);
	info.len = p_status->klen_array[0];

	ok = topo_cb(&info);
	if (false == ok){
		cache_add( p_cache, "-", 1 );
		cache_add( p_cache, info.erro, strnlen(info.erro, MAX_ERROR_MESSAGE_SIZE) );
		cache_add( p_cache, "\r\n", 2 );
		return -1; 
	}   
	if (0 == info.size){
		cache_add( p_cache, OPT_MULTI_BULK_NULL, strlen(OPT_MULTI_BULK_NULL) );
		return 0;
	}   
	// *2\r\n$5\r\nmykey\r\n$5\r\nmyval\r\n 
	int i = 0;
	cache_add( p_cache, "*", 1 );
	put_number_out(p_cache, info.size);
	for(; i < info.size; i++){
		cache_add( p_cache, "$", 1 );
		put_number_out( p_cache, get_number_len( info.buf[i] ) );
		put_number_out( p_cache, info.buf[i] );
	}
	return 0;
}


int entry_cmd_erbr(struct data_node *p_node, uint64_t idx)
{
	return entry_topo_main_call(p_node, idx, get_export_road_by_road, MAX_ONE_NODE_OWN_LINE_COUNT);
}

int entry_cmd_irbr(struct data_node *p_node, uint64_t idx)
{
	return entry_topo_main_call(p_node, idx, get_import_road_by_road, MAX_ONE_NODE_OWN_LINE_COUNT);
}

int entry_cmd_erof (struct data_node *p_node, uint64_t idx)
{
	return entry_topo_main_call(p_node, idx, get_export_road_of_node, MAX_ONE_NODE_OWN_LINE_COUNT);
}

int entry_cmd_irof (struct data_node *p_node, uint64_t idx)
{
	return entry_topo_main_call(p_node, idx, get_import_road_of_node, MAX_ONE_NODE_OWN_LINE_COUNT);
}

int entry_cmd_rlbr(struct data_node *p_node, uint64_t idx)
{
	return entry_topo_with_data_call(p_node, idx, get_road_list_by_road, MAX_ONE_NODE_OWN_LINE_COUNT * 2);
}

int entry_cmd_enbr(struct data_node *p_node, uint64_t idx)
{
	return entry_topo_main_call(p_node, idx, get_end_node_by_road, MAX_ONE_NODE_OWN_LINE_COUNT);
}

int entry_cmd_fnbr(struct data_node *p_node, uint64_t idx)
{
	return entry_topo_main_call(p_node, idx, get_from_node_by_road, MAX_ONE_NODE_OWN_LINE_COUNT);
}

