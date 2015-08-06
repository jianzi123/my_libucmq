#pragma once

#include <stdint.h>

#define MEM_STEP_PAGE_COUNT	32
#define GET_NEED_COUNT(idx, div)        ( (idx) / (div) +  ( ((idx) % (div) >= 1 )? 1 : 0) )


struct mem_list {
	int type_size;
	unsigned long peak_count;	/*max of full members*/
	unsigned long step_count;	/*max of bath members*/
	uintptr_t **ptr_slot;
};

struct mem_list *membt_init( int type_size, unsigned long peak_count );

int membt_good( struct mem_list *list, unsigned long index );

uintptr_t *membt_gain( struct mem_list *list, unsigned long index );
