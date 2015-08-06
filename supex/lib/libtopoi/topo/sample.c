#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "topo_api.h"

#if 0
int test_node_id_by_line_id(void)
{
	TP_LINE_OBJ *p_line = topo_pull_line( 76120306  );
	if (NULL == p_line){
		printf("\x1B[0;32m p_line is NULL\n \x1B[m");
		return -1;
	}
	TP_NODE_OBJ * p_node = p_line->goto_node;
	if (NULL == p_node){
		printf("\x1B[0;32m p_node is NULL\n \x1B[m");
		return -1;
	}
	printf("\x1B[0;32m p_node id is %zu\n \x1B[m", p_node->id);
	return 0;
}
#endif

int main(void)
{
	topo_start();
#if 0
	for(i=0; i<1000; i++) {
		optimalPath(&list, node, line, nodeMap, relMap, "105496100000001", "x05496100000006");
	}
	//printf("nextRelFromNode 2000 times: %ld usec\n", finish-start);
#endif
	return 0;
}
