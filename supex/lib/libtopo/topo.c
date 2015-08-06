/*
 * topo.c
 *
 *  Created on: Jun 19, 2014
 *      Author: buyuanyuan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdint.h>
#include <mysql.h>

#include "topo.h"
#include "util_map.h"


static struct mem_list *g_tp_node_list = NULL;
static struct mem_list *g_tp_line_list = NULL;

void topo_init(void)
{
	g_tp_node_list = membt_init( sizeof(TP_NODE_OBJ), MAX_TOPO_NODE_COUNT );
	g_tp_line_list = membt_init( sizeof(TP_LINE_OBJ), MAX_TOPO_LINE_COUNT );
	return;
}

struct mem_list *topo_fetch_list(int type)
{
	switch(type){
		case TYPE_TOPO_NODE:
			return g_tp_node_list;
		case TYPE_TOPO_LINE:
			return g_tp_line_list;	       
		default:
			return NULL;
	}
}

void topo_push_node(TP_NODE_OBJ *obj, uint64_t idx)
{
	TP_NODE_OBJ *p_node = (TP_NODE_OBJ *)membt_gain( g_tp_node_list, idx );
	memcpy(p_node, obj, sizeof(TP_NODE_OBJ));
	return;
}
TP_NODE_OBJ *topo_pull_node(uint64_t idx)
{
	if ( membt_good( g_tp_node_list, idx ) ){
		TP_NODE_OBJ *p_node = (TP_NODE_OBJ *)membt_gain( g_tp_node_list, idx );
		return ( (p_node->id > 0)?p_node:NULL );
	}else{
		return NULL;
	}
}

void topo_push_line(TP_LINE_OBJ *obj, uint64_t idx)
{
	TP_LINE_OBJ *p_line = (TP_LINE_OBJ *)membt_gain( g_tp_line_list, idx );
	memcpy(p_line, obj, sizeof(TP_LINE_OBJ));
	return;
}
TP_LINE_OBJ *topo_pull_line(uint64_t idx)
{
	if ( membt_good( g_tp_line_list, idx ) ){
		TP_LINE_OBJ *p_line = (TP_LINE_OBJ *)membt_gain( g_tp_line_list, idx );
		return ( (p_line->id > 0)?p_line:NULL );
	}else{
		return NULL;
	}
}

void topo_make(void)
{
	unsigned long i;
	for(i=0; i < MAX_TOPO_LINE_COUNT; i++) {
		if ( membt_good( g_tp_line_list, i ) ){
			TP_LINE_OBJ *p_line = (TP_LINE_OBJ *)membt_gain( g_tp_line_list, i );

			if( (p_line->id != i) || (!p_line->from_node) || (!p_line->goto_node) ){/*ignore id 0*/
				continue;
				//printf("\x1B[0;32m" "\tline [%zu] from [%x] goto [%x]\n" "\x1B[m", p_line->id, p_line->from_node, p_line->goto_node);
			}
			assert(p_line->from_node && p_line->goto_node);
			if( p_line->from_node->hept_line == NULL ) {
				p_line->from_node->hept_line = p_line;

				p_line->pfne_line = p_line;
				p_line->nfne_line = p_line;
			}else{
				TP_LINE_OBJ *p_head = p_line->from_node->hept_line;
				
				p_line->pfne_line = p_head->pfne_line;
				p_line->nfne_line = p_head;
			
				p_head->pfne_line->nfne_line = p_line;
				p_head->pfne_line = p_line;
			}
			if( p_line->goto_node->hipt_line == NULL ) {
				p_line->goto_node->hipt_line = p_line;

				p_line->pgni_line = p_line;
				p_line->ngni_line = p_line;
			}else{
				TP_LINE_OBJ *p_head = p_line->goto_node->hipt_line;

				p_line->pgni_line = p_head->pgni_line;
				p_line->ngni_line = p_head;
			
				p_head->pgni_line->ngni_line = p_line;
				p_head->pgni_line = p_line;
			}
		}
	}
	return;
}

/*=========================================================================*/
static void fetch_node_cb( MYSQL_ROW data, unsigned long *lens, void *args )
{
	TP_NODE_OBJ node = {
		.id = strtoull(data[0], NULL, 10),
		.longitude = atof(data[1]),
		.latitude = atof(data[2]),
		.hipt_line = NULL,
		.hept_line = NULL
	};
	assert( node.id > 0 );
//	printf("node : %zu", node.id);
	topo_push_node(&node, node.id);
	return;
}


static void fetch_line_cb( MYSQL_ROW data, unsigned long *lens, void *args )
{
	TP_LINE_OBJ line = {
		.id = strtoull(data[0], NULL, 10),
		.ban = 0,//TODO
		.direction = 0,

		.from_node = (struct tp_node_obj *)membt_gain( topo_fetch_list( TYPE_TOPO_NODE ), strtoull(data[1], NULL, 10) ),
		.goto_node = (struct tp_node_obj *)membt_gain( topo_fetch_list( TYPE_TOPO_NODE ), strtoull(data[2], NULL, 10) ),

		.pfne_line = NULL,
		.nfne_line = NULL,

		.pgni_line = NULL,
		.ngni_line = NULL
	};

	double from_lon = line.from_node->longitude;
	double from_lat = line.from_node->latitude;
	double to_lon   = line.goto_node->longitude;
	double to_lat   = line.goto_node->latitude;
	line.direction = util_get_angle(from_lon, from_lat, to_lon,to_lat);
	line.length = util_get_length(from_lon, from_lat, to_lon,to_lat);

	assert( line.id > 0 );
	//	printf("line : %zu", line.id);
	topo_push_line(&line, line.id);
	return;
}

#define SQL_REQUIRE_STEP	10000
#define SQL_NODE_QUERY		"SELECT nodeID, longitude, latitude FROM nodeInfo where nodeID between %d and %d"
#define SQL_LINE_QUERY 		"SELECT roadID, fNodeID, eNodeID FROM arcInfo where roadID between %d and %d"


void topo_load( struct sql_info *info_node, struct sql_info *info_line )
{
	char sql_buf[256] = {};
	int64_t ret = 0;

	/*fetch node data first*/
	info_node->sql = sql_buf;
	int id = 10000000;
	while( id < 50000000){
		memset(sql_buf, 0, 256);
		sprintf(sql_buf, SQL_NODE_QUERY, id, id + SQL_REQUIRE_STEP);
		
		ret = mysql_cmd( info_node, fetch_node_cb, NULL );

		id += SQL_REQUIRE_STEP;
	}
	
	/*fetch line data second*/
	info_line->sql = sql_buf;
	id = 70000000;
	while( id < 87000000){
		memset(sql_buf, 0, 256);
		sprintf(sql_buf, SQL_LINE_QUERY, id, id + SQL_REQUIRE_STEP);
		
		ret = mysql_cmd( info_line, fetch_line_cb, NULL );

		id += SQL_REQUIRE_STEP;
	}
	return;
}
