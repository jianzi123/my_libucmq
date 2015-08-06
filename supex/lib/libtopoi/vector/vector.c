#include<string.h>
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<mysql.h>

#include"vector.h"

#if 1 
	int count =0 ;
#endif 
static struct mem_list * g_lrp_poi_list = NULL;
static struct mem_list * g_lrp_line_list = NULL;

void lrp_init(void) {

    	g_lrp_poi_list = membt_init(sizeof(LRP_POI_OBJ),MAX_LRP_POI_COUNT);
	g_lrp_line_list = membt_init(sizeof(LRP_LINE_OBJ),MAX_LRP_LINE_COUNT);
	return;
}
    
struct mem_list * lrp_fetch_list(int type) {

    	switch(type) {
		case TYPE_LRP_POI:
		    	return g_lrp_poi_list;
		case TYPE_LRP_LINE:
			return g_lrp_line_list;
		default:
			return NULL;
	}
}

void lrp_push_poi(LRP_POI_OBJ * obj , uint64_t idx ) {

	LRP_POI_OBJ * p_poi = (LRP_POI_OBJ *)membt_gain(g_lrp_poi_list,idx);
	memcpy(p_poi,obj,sizeof(LRP_POI_OBJ));
	return ;
}

LRP_POI_OBJ * lrp_pull_poi(uint64_t idx ) {

    	if ( membt_good(g_lrp_poi_list,idx) ) {

		LRP_POI_OBJ * p_poi = (LRP_POI_OBJ *)membt_gain(g_lrp_poi_list,idx);
		return ( (p_poi->poi_id !=NULL)?p_poi:NULL );
	}else{

	    return NULL;
	}
}

void lrp_push_line(LRP_LINE_OBJ * obj ,uint64_t idx) {
	
    	LRP_LINE_OBJ * p_line = (LRP_LINE_OBJ *)membt_gain(g_lrp_line_list,idx);
	memcpy(p_line,obj,sizeof(LRP_LINE_OBJ));
	return;
}

LRP_LINE_OBJ * lrp_pull_line(uint64_t idx) {

    	if ( membt_good(g_lrp_line_list,idx/1000) ) {

		LRP_LINE_OBJ * p_line = (LRP_LINE_OBJ *)membt_gain(g_lrp_line_list,idx/1000);
		return ( (p_line->line_id >0)?p_line:NULL );
	}else{

	    return NULL;
	}
}

void lrp_make(void) {
    	unsigned long i ;
	for(i=0 ;i< MAX_LRP_POI_COUNT;i++){ //fix 
	    	if ( membt_good(lrp_fetch_list(TYPE_LRP_POI),i) ){
			LRP_POI_OBJ * p_poi = (LRP_POI_OBJ*)membt_gain(lrp_fetch_list(TYPE_LRP_POI),i);
			if (membt_good(g_lrp_line_list,p_poi->line_id/1000)){
			    	LRP_LINE_OBJ * p_line = (LRP_LINE_OBJ *)membt_gain(g_lrp_line_list,p_poi->line_id/1000);
				if (p_line->p_head  == NULL) {
					p_line->p_head = p_poi;
					p_line->p_end = p_poi;
					
				}else {
			    		
					p_line->p_end->next = p_poi;
					p_line->p_end = p_poi;
					
				}
				p_line->poi_num++;
				
			}

			//p_line->poi_list[(p_line->poi_num)]= (LRP_POI_OBJ *)calloc(1,sizeof(LRP_POI_OBJ*));
			//p_line->poi_list[(p_line->poi_num)++]= p_poi;

		}
		//printf("%d\n",i);
	}
	printf(">>>>>>>>the lrp has been made\n");
		
}

static void fetch_poi_line_cb(MYSQL_ROW data ,unsigned long * lens , void * args) {
	LRP_POI_OBJ poi = {
	    	
		.latitude  = atof(data[2]),
		.longitude = atof(data[1]),

		.angle1 = atoi(data[3]),
		.angle2 = atoi(data[4]),
		
		.line_id = strtoull((data[5]),NULL,10),
		.type = atoi(data[6]),
		//.line = (LRP_LINE_OBJ *)membt_gain(lrp_fetch_list(TYPE_LRP_LINE),strtoull(data[5],NULL,10)),
		.next=NULL,
	};
		
		poi.poi_id = calloc(1,10);
		memset(poi.poi_id,0,10);
		strcpy(poi.poi_id , data[0]);

		lrp_push_poi(&poi , strtoull(poi.poi_id+1,NULL,10));
		//printf("\tload poi_id:%d\t count:%d \n",strtoull(poi.poi_id+1,NULL,10),count++);

	LRP_LINE_OBJ line = {
		.line_id = strtoull(data[5],NULL,10)/1000,
		.poi_num = 0 ,
		.p_head = NULL,
		
		
	};
	//if (!membt_good(lrp_fetch_list(TYPE_LRP_LINE),line.line_id)) {
	//	lrp_push_line(&line,line.line_id);/* only push one time ,avoid many time push */
	//	printf("\tload line_id :%ld\n",line.line_id);
	//}
		lrp_push_line(&line,line.line_id);
		//printf("\tload line_id :%ld\n",line.line_id);
	
	return ;
}

#define SQL_POI_QUERY  "SELECT ID, B, L , angle1, angle2, lineId, type  FROM POIInfo  LIMIT %d , %d"
void lrp_load(struct sql_info * info_line_poi) {
	char sql_buf[256] = {};
	uint64_t ret = 0;
	info_line_poi->sql = sql_buf;
	int  id = 0;
	while( 1 ) {
		memset(sql_buf,0,256);
		sprintf(sql_buf,SQL_POI_QUERY,id,10000);
		ret = mysql_cmd(info_line_poi,fetch_poi_line_cb,NULL);
		if (ret < 10000) break;
		id += 10000;
	}
	printf(">>>>>>the lrp has been loaded\n");
	return ;
}





