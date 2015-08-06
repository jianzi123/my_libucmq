#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/********************************QUEUE**************************************/
void item_init(CQ_ITEM *item) {
	item->next = NULL;
}
/*
 * Initializes a connection queue.
 */
void cq_init(CQ_LIST *list) {
	X_LOCK_INIT( &list->lock );
	list->nubs = 0;
	list->head = NULL;
	list->tail = NULL;
}

/*
 * Looks for an item on a connection queue, but doesn't block if there isn't
 * one.
 * Returns the item, or NULL if no item is available
 */
CQ_ITEM *cq_pop(CQ_LIST *list) {
	X_LOCK( &list->lock );
	CQ_ITEM *item = list->head;
	if (item != NULL) {
		list->head = item->next;
		if (list->head == NULL)
			list->tail = NULL;
		list->nubs--;
	}
	X_UNLOCK( &list->lock );
	return item;
}


/*
 * Adds an item to a connection queue.
 */
void cq_push(CQ_LIST *list, CQ_ITEM *item) {
	item->next = NULL;
	X_LOCK( &list->lock );
	if (list->tail == NULL)
		list->head = item;
	else
		list->tail->next = item;
	list->tail = item;
	list->nubs++;
	X_UNLOCK( &list->lock );
}

int cq_view(CQ_LIST *list) {
	return list->nubs;
}

