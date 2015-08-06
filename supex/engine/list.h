#pragma once

#include <pthread.h>

#include "utils.h"


/* An item in the connection queue. */
typedef struct queue_item CQ_ITEM;
struct queue_item {
	void *data;
	struct queue_item *next;
};


/* A connection queue. */
typedef struct queue_list CQ_LIST;
struct queue_list {
	int nubs;
	CQ_ITEM *head;
	CQ_ITEM *tail;
	X_NEW_LOCK lock;
};

void item_init(CQ_ITEM *item);
void cq_init(CQ_LIST *list);
CQ_ITEM *cq_pop(CQ_LIST *list);
void cq_push(CQ_LIST *list, CQ_ITEM *item);
int cq_view(CQ_LIST *list);
