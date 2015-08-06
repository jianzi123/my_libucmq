#pragma once

#include "vector.h"
#include "common.h"
#ifndef true 
#define true  1 
#endif 

#ifndef false 
#define false 0 
#endif 
 
//#define MAX_ERROR_MESSAGE_SIZE            32
//#define ERROR_QUERY_NO_ID              "NO THIS ID %zu"
//#define ERROR_PEAK_IS_SMALL            "PEAK IS SMALL"

#ifdef TEST_ONLY   
void lrp_start(void);
#else 
void lrp_start(char * conf);
#endif

/*struct query_args { 
	//char  args[1024];
	uint64_t idx;
	uint64_t * buf;
	char data[32];
	int len ;
	int peak ;
	int size ;
	char erro[MAX_ERROR_MESSAGE_SIZE];
};*/

typedef int (* LRP_CALLBACK)(struct query_args *info );

int get_poi_id_by_line(struct query_args * info );

int get_poi_info_by_poi(struct query_args * info );

