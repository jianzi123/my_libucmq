#pragma once

#include<stdint.h>
#include<unistd.h>

#include"memory_bath.h"
#include"sql_api.h"

#define MAX_LRP_POI_COUNT  100000000
#define MAX_LRP_LINE_COUNT 100000000

#define TYPE_LRP_POI  0 
#define TYPE_LRP_LINE 1


typedef struct poi {
    	//uint64_t poi_id;
	char * poi_id;
	uint64_t line_id;
	double longitude;
	double latitude;

	int  angle1; 
	int  angle2;

	uint64_t type; /*poi type*/

	struct poi * next; 
} LRP_POI_OBJ;

typedef struct line {

    	uint64_t line_id ;
	uint32_t poi_num;/* the num of poi */
    	struct poi * p_head;/* the head of poi list */
	struct poi * p_end;/*the end of poi list */

} LRP_LINE_OBJ;


struct mem_list * lrp_fetch_list(int type);

void lrp_push_poi(LRP_POI_OBJ * obj ,uint64_t idx);
LRP_POI_OBJ * lrp_pull_poi(uint64_t idx);

void lrp_push_line(LRP_LINE_OBJ * obj,uint64_t idx);
LRP_LINE_OBJ * lrp_pull_line(uint64_t idx);

void lrp_init(void);

void lap_make(void);

void lrp_load(struct sql_info * info_poi);
