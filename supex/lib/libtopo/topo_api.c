/*
 * interface.c
 *
 *  Created on: Jun 28, 2014
 *      Author: buyuanyuan
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "topo_api.h"

#ifndef TEST_ONLY
#include "topo_cfg.h"
#endif

#ifdef TEST_ONLY
void topo_start(void)
{
	struct sql_info info_nb1 = {
		.host = "192.168.1.10",
		.port = 3306,
	        .username = "observer",
		.password = "abc123",
		.database = "roadMap",
		.sql = NULL
	};
	struct sql_info info_nb2 = {
		.host = "192.168.1.10",
		.port = 3306,
	        .username = "observer",
		.password = "abc123",
		.database = "roadMap",
		.sql = NULL
	};
#else
void topo_start(char *conf)
{
	struct topo_cfg_file g_topo_cfg_file = {};
	read_topo_cfg(&g_topo_cfg_file, conf);
	struct sql_info info_nb1 = {
		.host = g_topo_cfg_file.node_host,
		.port = g_topo_cfg_file.node_port,
	        .username = g_topo_cfg_file.node_username,
		.password = g_topo_cfg_file.node_password,
		.database = g_topo_cfg_file.node_database,
		.sql = NULL
	};
	struct sql_info info_nb2 = {
		.host = g_topo_cfg_file.line_host,
		.port = g_topo_cfg_file.line_port,
	        .username = g_topo_cfg_file.line_username,
		.password = g_topo_cfg_file.line_password,
		.database = g_topo_cfg_file.line_database,
		.sql = NULL
	};
#endif
	topo_init( );
	topo_load( &info_nb1, &info_nb2 );
	topo_make( );
	return;
}

int get_export_road_by_road(struct query_args *info)
{
	TP_LINE_OBJ *p_line = topo_pull_line( info->idx );
	if (NULL == p_line){
		snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_QUERY_NO_ID, info->idx);
		return false;
	}
	TP_LINE_OBJ *p_head = p_line->goto_node->hept_line;
	if (NULL == p_head){
		info->size = 0;
		return true;
	}
	p_line = p_head;
	int i = 0;
	do{
		if (i >= info->peak){
			snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_PEAK_IS_SMALL);
			return false;
		}
		info->buf[ i ] = p_line->id;
		info->size = ++i;

		p_line = p_line->nfne_line;
	}while(p_line != p_head);
	return true;
}
int get_import_road_by_road(struct query_args *info)
{
	TP_LINE_OBJ *p_line = topo_pull_line( info->idx );
	if (NULL == p_line){
		snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_QUERY_NO_ID, info->idx);
		return false;
	}
	TP_LINE_OBJ *p_head = p_line->from_node->hipt_line;
	if (NULL == p_head){
		info->size = 0;
		return true;
	}
	p_line = p_head;
	int i = 0;
	do{
		if (i >= info->peak){
			snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_PEAK_IS_SMALL);
			return false;
		}
		info->buf[ i ] = p_line->id;
		info->size = ++i;

		p_line = p_line->ngni_line;
	}while(p_line != p_head);
	return true;
}

TP_LINE_OBJ * get_next_most_straight_line(TP_LINE_OBJ *p_line, double dir) {
    TP_LINE_OBJ *p_head = p_line->goto_node->hept_line;
    TP_LINE_OBJ *p_tag = NULL;
    double min = 500;
    if (NULL == p_head){
        return NULL;
    }
    p_line = p_head;
    do {
        if (abs(p_line->direction - dir ) < min) {
            min = abs(p_line->direction - dir );
            p_tag = p_line;
        }
        p_line = p_line->nfne_line;
    } while (p_head != p_line);

    return p_tag;
}

int get_road_list_by_road(struct query_args *info)
{
    int         i, meter, threshold;
    uint64_t    road_id;
    char *      ptr;
    char        tmp [32];
    int         line_len;
    double      dir;
    memset(tmp, 0, 32);
    strncpy(tmp, info->data, info->len);
    ptr = strchr(tmp, ':');
    road_id = atoi(ptr + 1);
    info->idx = road_id;
    ptr = strchr(ptr + 1, ':');
    meter = atoi(ptr + 1);
    threshold = 2000; 

    /*
    fprintf(stdout, "[tmp = %s]\n", tmp);
    fprintf(stdout, "[road_id = %d]\n", road_id);
    fprintf(stdout, "[meter = %d]\n", meter);
    */

    TP_LINE_OBJ *p_line = topo_pull_line( info->idx );
    if (NULL == p_line){
        snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_QUERY_NO_ID, info->idx);
        return false;
    }
    dir = p_line->direction;

    info->size = 0;
    line_len = p_line->length - meter;
    threshold = threshold - line_len;
    if (threshold < 0 ) {
        info->buf[0] = p_line->id;
        info->buf[1] = threshold;
    } else {
        info->buf[0] = p_line->id;
        info->buf[1] = line_len;
    }
    info->size += 2;

    i = 1;
    while (threshold > 0) {
        p_line = get_next_most_straight_line(p_line, dir);
        if (p_line == NULL)  {
            info->size = 0;
            return true;
        }
        line_len = p_line->length;
        threshold = threshold - line_len;
        if (threshold < 0 ) {
            info->buf[i * 2] = p_line->id;
            info->buf[i * 2 + 1] = threshold;
        } else {
            info->buf[i * 2 ] = p_line->id;
            info->buf[i * 2 + 1] = line_len;
        }
        info->size += 2;
    }

    /*
    info->size = 10;
    i = 0;
    for(i = 0; i < 5; i++) {
        info->buf[i * 2] = 123232 + i;
        info->buf[i * 2 + 1] = 12+ i;
    }
    */

    return true;
}



int get_export_road_of_node(struct query_args *info)
{
    TP_NODE_OBJ * p_node = topo_pull_node( info->idx);
	if (NULL == p_node) {
		snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_QUERY_NO_ID, info->idx);
		return false;
	}
	TP_LINE_OBJ *p_head = p_node->hept_line;
	if (NULL == p_head){
		info->size = 0;
		return true;
	}
	TP_LINE_OBJ * p_line = p_head;
	int i = 0;
	do{
		if (i >= info->peak){
			snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_PEAK_IS_SMALL);
			return false;
		}
		info->buf[ i ] = p_line->id;
		info->size = ++i;

		p_line = p_line->nfne_line;
	}while(p_line != p_head);
	return true;
}

int get_import_road_of_node(struct query_args *info)
{
    TP_NODE_OBJ * p_node = topo_pull_node( info->idx);
	if (NULL == p_node){
		snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_QUERY_NO_ID, info->idx);
		return false;
	}
	TP_LINE_OBJ *p_head = p_node->hipt_line;
	if (NULL == p_head){
		info->size = 0;
		return true;
	}
	TP_LINE_OBJ* p_line = p_head;
	int i = 0;
	do{
		if (i >= info->peak){
			snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_PEAK_IS_SMALL);
			return false;
		}
		info->buf[ i ] = p_line->id;
		info->size = ++i;

		p_line = p_line->ngni_line;
	}while(p_line != p_head);
	return true;

}

int get_end_node_by_road(struct query_args *info)
{
	TP_LINE_OBJ *p_line = topo_pull_line( info->idx );
	if (NULL == p_line){
		snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_QUERY_NO_ID, info->idx);
		return false;
	}
    TP_NODE_OBJ * p_node = p_line->goto_node;
	if (NULL == p_node){
		snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_QUERY_NO_ID, info->idx);
		return false;
	}
    info->buf[0] = p_node->id;
    info->size = 1;
    return true;
}

int get_from_node_by_road(struct query_args *info)
{
	TP_LINE_OBJ *p_line = topo_pull_line( info->idx );
	if (NULL == p_line){
		snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_QUERY_NO_ID, info->idx);
		return false;
	}
    TP_NODE_OBJ * p_node = p_line->from_node;
	if (NULL == p_node){
		snprintf(info->erro, MAX_ERROR_MESSAGE_SIZE, ERROR_QUERY_NO_ID, info->idx);
		return false;
	}
    info->buf[0] = p_node->id;
    info->size = 1;
    return true;
}


/*

get_path_from_node_to_node()
{

}
get_path_from_road_to_node()
{

}




get_path_from_node_on_road()
{

}
get_path_from_road_on_road()
{

}



static double getCos(TP_NODE_OBJ *nr1, TP_NODE_OBJ *nr2, TP_NODE_OBJ *nr3)
{
	Position *p1 = NULL;
	Position *p2 = NULL;
	Position *p3 = NULL;
	p1 = (Position *)malloc(sizeof(Position));
	p2 = (Position *)malloc(sizeof(Position));
	p3 = (Position *)malloc(sizeof(Position));
	if(!p1 || !p2 || !p3) {
		printf("malloc error!\n");
		exit(-1);
	}
	p1->lon = nr1->longitude;
	p1->lat = nr1->latitude;
	p2->lon = nr2->longitude;
	p2->lat = nr2->latitude;
	p3->lon = nr3->longitude;
	p3->lat = nr3->latitude;
	return get_cos(p1, p2, p3);
}

static uint64_t optimalNode(TP_NODE_OBJ *nodeRecords, TP_LINE_OBJ *relationshipRecords, RBT *nodeMap, RBT *relMap, char *firstNode, char *secondNode)
{
	List *list = NULL;
	list_init(&list);
	nextRelFromNode(&list, nodeRecords, relationshipRecords, nodeMap, firstNode);
	uint64_t i, tempNodeId, tempRelId;
	uint64_t firstNodeId = search(nodeMap, firstNode);
	uint64_t secondNodeId = search(nodeMap, secondNode);
	TP_NODE_OBJ *optimalNode = NULL;
	double temp = -1.0;
	double temp2;
	optimalNode = (TP_NODE_OBJ *)malloc(sizeof(TP_NODE_OBJ));
	if(!optimalNode) {
		printf("malloc error!\n");
		exit(-1);
	}
	for(i=0; i<list->size; i++) {
		tempRelId = search(relMap, list_get(list, i));
		tempNodeId = relationshipRecords[tempRelId].goto_node;
		temp2 = getCos(&(nodeRecords[firstNodeId]), &(nodeRecords[secondNodeId]), &(nodeRecords[tempNodeId]));
		if(temp2 > temp) {
			temp = temp2;
			optimalNode = &(nodeRecords[tempNodeId]);
		}
	}
	return optimalNode->id;
}

void optimalPath(List **list, TP_NODE_OBJ *nodeRecords, TP_LINE_OBJ *relationshipRecords, RBT *nodeMap, RBT *relMap, char *firstNode, char *secondNode)
{
	uint64_t secondNodeId = search(nodeMap, secondNode);
	uint64_t tempNodeId = -1;
	list_init(list);
	list_add(*list, firstNode);
	while(tempNodeId != secondNodeId) {
		tempNodeId = optimalNode(nodeRecords, relationshipRecords, nodeMap, relMap, firstNode, secondNode);
		list_add(*list, nodeRecords[tempNodeId].node_id);
		firstNode = nodeRecords[tempNodeId].node_id;
	}
}
*/
