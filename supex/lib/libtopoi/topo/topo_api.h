/*
 * interface.h
 *
 *  Created on: Jun 28, 2014
 *      Author: buyuanyuan
 */
#pragma once

#include "topo.h"
#include "common.h"
#ifndef false
#define false		0
#endif
#ifndef true
#define true		1
#endif

//#define MAX_ERROR_MESSAGE_SIZE		32

//#define ERROR_QUERY_NO_ID	"No this id %zu"
//#define ERROR_PEAK_IS_SMALL	"Peak is small"

#ifdef TEST_ONLY
void topo_start(void);
#else
void topo_start(char *conf);
#endif

/*struct query_args {
    uint64_t idx;
    uint64_t *buf;
    char data[32];
    int len;
    int peak; 
    int size;
    char erro[ MAX_ERROR_MESSAGE_SIZE ];
};*/

typedef int (*TOPO_CALLBACK)(struct query_args *info);


int get_export_road_by_road(struct query_args *info);

int get_import_road_by_road(struct query_args *info);

int get_export_road_of_node(struct query_args *info);

int get_import_road_of_node(struct query_args *info);

int get_road_list_by_road(struct query_args *info);

int get_end_node_by_road(struct query_args *info);

int get_from_node_by_road(struct query_args *info);

