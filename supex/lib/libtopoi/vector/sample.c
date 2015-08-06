#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "vector_api.h"

#if 0
int test_poi_id_by_line(void)
{
	LRP_LINE_OBJ *p_line = lrp_pull_line(74845461000);
	if (NULL == p_line){
		printf("\x1B[0;32m p_line is NULL\n \x1B[m");
		return -1;
	}
	int i =0;
	LRP_POI_OBJ * p_poi = p_line->p_head;
	if (NULL == p_poi){
		printf("\x1B[0;32m p_poi is NULL\n \x1B[m");
		return -1;
	}
	while (p_poi != NULL) {
	printf("\x1B[0;32m p_node id is %zu\n \x1B[m", strtoull(p_poi->poi_id + 1,NULL,10));
	p_poi = p_poi->next;
	}
	return  0;
}


int test_poi_type_by_poi(void)
{
    LRP_POI_OBJ * p_poi = lrp_pull_poi(139);
	if (NULL == p_poi){
		printf("\x1B[0;32m p_poi is NULL\n \x1B[m");
		return -1;
	}else{
	printf("\x1B[0;32m poi type is %zu\n \x1B[m", p_poi->type);

	}

}
   
#endif



int main(void)
{
	lrp_start();
#if 0
	 test_poi_id_by_line();
	 test_poi_type_by_poi();
#endif 
#if 0
	for(i=0; i<1000; i++) {
		optimalPath(&list, node, line, nodeMap, relMap, "105496100000001", "x05496100000006");
	}
	//printf("nextRelFromNode 2000 times: %ld usec\n", finish-start);
#endif
	return 0;
}
