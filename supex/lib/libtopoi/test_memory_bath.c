#include <stdio.h>

#include "memory_bath.h"

struct abc {
	int a;
	int b;
};

int main()
{

	struct mem_list *list = membt_init( sizeof(struct abc), 100000000 );

	struct abc *x = (struct abc *)membt_gain( list, 10000005 );
	
	struct abc *y = (struct abc *)membt_gain( list, 10000006 );
	
	struct abc *z = (struct abc *)membt_gain( list, 20000000 );


	printf("x %x, y %x, z %x\n", x, y, z);
	return 0;
}
