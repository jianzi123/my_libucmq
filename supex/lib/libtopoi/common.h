#pragma once


 
#define MAX_ERROR_MESSAGE_SIZE            32
#define ERROR_QUERY_NO_ID              "NO THIS ID %zu"
#define ERROR_PEAK_IS_SMALL            "PEAK IS SMALL"


struct query_args { 
	//char  args[1024];
	uint64_t idx;
	uint64_t * buf;
	char data[32];
	int len ;
	int peak ;
	int size ;
	char erro[MAX_ERROR_MESSAGE_SIZE];
};




