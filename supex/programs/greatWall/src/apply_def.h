#pragma once

#include "sniff_api.h"

#define OVERLOOK_DELAY_LIMIT	3

enum {
	REAL_TIME_KIND = 0,
	NON_REAL_TIME_KIND,
	LIMIT_CHANNEL_KIND,
};


struct mount_info {
	PARTER_PTHREAD *list;
	struct mount_info *next;
};


#define MAX_SNIFF_LABEL_LENGTH			64
#define MAX_SNIFF_FLOWS_LENGTH			(MAX_SNIFF_DATA_SIZE - MAX_SNIFF_LABEL_LENGTH)

struct route_msg_data {
	char label[ MAX_SNIFF_LABEL_LENGTH ];
	char flows[ MAX_SNIFF_FLOWS_LENGTH ];
};
