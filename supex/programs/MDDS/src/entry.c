#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "entry.h"
#include "net_cache.h"
#include "topo/topo.h"
#include "topo/topo_api.h"
#include "vector/vector.h"
#include "vector/vector_api.h"
#include "get_poi.h"

/*
   if ( ret == X_MALLOC_FAILED ){
   clean_send_node(p_node);
   add_send_node(p_node, OPT_NO_MEMORY, strlen(OPT_NO_MEMORY));
   return X_MALLOC_FAILED;
   }
   */

void entry_init(void)
{
	topo_start( "mdds_conf.json" );
	lrp_start( "mdds_conf.json" );
#if 0
	char tmp [1024];
	int len;
	while ( 1 == 1) {
		//scanf("%s", tmp);
		memset(tmp, 0, 1024);
		fgets(tmp, 1024, stdin );
		len = strlen(tmp);
		if ( len == 6 && strncmp(tmp, "login", 5) == 0)
			break;
		//fprintf(stdout, "[tmp = %s]\n", tmp);
	}
#else
	char shell[1024] = {0};
	//char format[] = "unset kid; for pid in `pidof topology`; do if [ $pid -ne %d ]; then kid+=$pid" "; fi done; if [ $kid ]; then kill -9 $kid; fi";
	char format[] = "unset kid; for pid in `pidof mdds`; do kid+=${pid//%d/}" "; done; if [ $kid ]; then kill -9 $kid; fi";
	sprintf(shell, format, getpid());
	system( shell );
#endif

#if 0 			
		int i =0;
		uint64_t line[2]={74839724003,74838040001};
		int len_of_line = sizeof(line)/sizeof(uint64_t);
		uint64_t poi[100];
		int num_poi  = get_poi_by_line_set(line,len_of_line,poi);
		printf("the num of poi :%d\n",num_poi);
		uint64_t res[num_poi][6];
		int res_num = get_poi_info_by_poi_set(poi,num_poi,res);
		for (i=0;i<res_num;i++) {
		printf("test ---- result %ld,%ld,%ld,%ld,%ld,%ld\n",res[i][0],res[i][1],res[i][2],res[i][3],res[i][4],res[i][5]);}


#endif
}
