#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "memory_bath.h"



struct mem_list *membt_init( int type_size, unsigned long peak_count )
{
	struct mem_list *list = calloc(1, sizeof(struct mem_list));
	assert( list );

	printf("\x1B[0;32mPEAK_COUNT\t=\t\x1B[1;31m%zu\n\x1B[m", peak_count);
	unsigned long page_size = getpagesize();
	unsigned long bath_size = page_size * MEM_STEP_PAGE_COUNT;
	unsigned long step_count = bath_size / type_size;

	list->type_size = type_size;
	list->peak_count = peak_count;
	list->step_count = step_count;
	list->ptr_slot = calloc( GET_NEED_COUNT(peak_count, step_count), sizeof(uintptr_t *) );
	assert( list->ptr_slot );

	return list;
}

uintptr_t *membt_gain( struct mem_list *list, unsigned long index )
{
	//printf("%llu\n", index);
	assert( index >= 0 && index < list->peak_count );
	unsigned long divisor	= (index + 1) / list->step_count;
	unsigned long remainder	= (index + 1) % list->step_count;
	unsigned long offset = ( divisor + ( (remainder >= 1) ? 1 : 0 ) ) - 1;
	uintptr_t **base = &list->ptr_slot[ offset ];
	if (*base == NULL){
		*base = calloc( MEM_STEP_PAGE_COUNT,  getpagesize() );
		assert( *base );
	}
	if ( remainder == 0 ){
		remainder = list->step_count;
	}
	remainder --;
	uintptr_t *addr = (uintptr_t *)&( ((char *)*base)[ list->type_size * remainder ] );
	return addr;
}

int membt_good( struct mem_list *list, unsigned long index )
{
	if( index < 0 || index >= list->peak_count ){
		return !!(NULL);
	}
	assert( index >= 0 && index < list->peak_count );
	
	unsigned long offset = GET_NEED_COUNT(index + 1, list->step_count) - 1;
	
	return ( !!list->ptr_slot[ offset ] );
}
