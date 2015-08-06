/*
 * topo.h
 *
 *  Created on: Jun 19, 2014
 *      Author: buyuanyuan
 */

#pragma once

#include <stdint.h>
#include <unistd.h>

#include "memory_bath.h"
#include "sql_api.h"

#define MAX_TOPO_NODE_COUNT	100000000
#define MAX_TOPO_LINE_COUNT	100000000
#define MAX_ONE_NODE_OWN_LINE_COUNT	8

#define TYPE_TOPO_NODE		0
#define TYPE_TOPO_LINE		1

typedef struct tp_node_obj {
	uint64_t id;

	double longitude;
	double latitude;

	struct tp_line_obj *hipt_line;	/*head import line*/
	struct tp_line_obj *hept_line;	/*head export line*/
} TP_NODE_OBJ;

typedef struct tp_line_obj {
	uint64_t id;
	uint64_t ban;		/*we can use bit to record the index which is ban when foreach the list*/
	double direction;
	uint64_t length;

	struct tp_node_obj *from_node;
	struct tp_node_obj *goto_node;

	struct tp_line_obj *pfne_line;	/*prev from_node's export line*/
	struct tp_line_obj *nfne_line;	/*next from_node's export line*/

	struct tp_line_obj *pgni_line;	/*prev goto_node's import line*/
	struct tp_line_obj *ngni_line;	/*next goto_node's import line*/
} TP_LINE_OBJ;


struct mem_list *topo_fetch_list(int type);

void topo_push_node(TP_NODE_OBJ *obj, uint64_t idx);
TP_NODE_OBJ *topo_pull_node(uint64_t idx);

void topo_push_line(TP_LINE_OBJ *obj, uint64_t idx);
TP_LINE_OBJ *topo_pull_line(uint64_t idx);

void topo_init(void);

void topo_make(void);

void topo_load( struct sql_info *info_node, struct sql_info *info_line );
