#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"vector_api.h"
#ifndef TEST_ONLY 
#include"vector_cfg.h"
#endif 

#ifdef TEST_ONLY 
void lrp_start(void){
	struct sql_info  info_nb = {
		.host = "192.168.1.21",
		.port =  3306,
		.username = "observer",
		.password = "abc123",
		.database = "mapdata",
		.sql = NULL,

	
	};
#else 
void lrp_start(char * conf ){
	struct lrp_cfg_file g_lrp_cfg_file = {};
	read_lrp_cfg(&g_lrp_cfg_file ,conf);
	struct sql_info  info_nb ={
	    .host =g_lrp_cfg_file.poi_host,
	    .port = g_lrp_cfg_file.poi_port,
	    .username = g_lrp_cfg_file.poi_username,
	    .password = g_lrp_cfg_file.poi_password,
	    .database = g_lrp_cfg_file.poi_database,
	    .sql = NULL,
	};

#endif

	lrp_init();
	lrp_load(&info_nb);
	lrp_make();
	return ;
}

int get_poi_id_by_line(struct query_args * info ){
		
		LRP_LINE_OBJ * p_line = lrp_pull_line(info->idx);
	
		if (NULL == p_line) {
	    	snprintf(info->erro,MAX_ERROR_MESSAGE_SIZE,ERROR_QUERY_NO_ID,info->idx);
	    	return false ;
		}
		LRP_POI_OBJ * temp = p_line->p_head;
		printf("get poi is not nil\n");
		int idx =0;
		while(temp != NULL){
	    		 
			info->buf[idx]= strtoull(temp->poi_id+1,NULL,10);
			idx++; 
			
			temp= temp->next;
		}	
	
		info->size = idx;
		return true;
}

int get_poi_info_by_poi(struct query_args * info ){
	LRP_POI_OBJ * poi = lrp_pull_poi(info->idx);
	if (NULL == poi ) {
		snprintf(info->erro,MAX_ERROR_MESSAGE_SIZE,ERROR_QUERY_NO_ID,info->idx);
		return false;
	}
	info->buf[0] = poi->type;
	info->buf[1] = poi->longitude*1000000;
	info->buf[2] = poi->latitude*1000000;
	info->buf[3] = poi->angle1 + 360;
	info->buf[4] = poi->angle2 + 360;
	info->size = 5;
	return true ;
}



