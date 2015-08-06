#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include <stdlib.h>

#include "topo/topo_api.h"
#include "vector/vector_api.h"
#include "topo/topo.h"
#include "vector/vector.h"
#include "get_poi.h"

static int  get_poi_info_by_poi_set(uint64_t *poi_set,int len ,uint64_t (*res)[6]) {
    	
	int idx=0;
	int res_idx=0;
	for (idx=0 ; idx<len ; idx++) {
		LRP_POI_OBJ * p_poi = lrp_pull_poi(poi_set[idx]);
		if (p_poi == NULL ) {
			/*error process*/
		}else{
			res[res_idx][0]=poi_set[idx];
			res[res_idx][1]=p_poi->type;
			res[res_idx][2]=p_poi->longitude*1000000;
			res[res_idx][3]=p_poi->latitude*1000000;
			res[res_idx][4]=p_poi->angle1+360;
			res[res_idx][5]=p_poi->angle2+360;
			++res_idx;
		}
	}
	return res_idx;
    	
}

static int  get_poi_by_line_set(uint64_t * line_set,int len ,uint64_t * poi_set) {
	
	int idx=0;
	int poi_idx=0;
	for (idx = 0; idx < len;idx++) {
		LRP_LINE_OBJ * p_line = lrp_pull_line(line_set[idx]);
		if(p_line!=NULL) {
			LRP_POI_OBJ * p_poi= p_line->p_head;
			while(p_poi != NULL && poi_idx<100) { //fix  
			    	if (line_set[idx]==p_poi->line_id){
				poi_set[poi_idx]=strtoull(p_poi->poi_id+1 ,NULL ,10);
				++poi_idx;}
				p_poi = p_poi->next;
			}
		}else{
			/*error process*/
		}
	}
	return poi_idx;

}

static int get_front_road_by_road(uint64_t road_id,uint64_t * res)
{
    	int count=0;
	TP_LINE_OBJ *p_line = topo_pull_line( road_id );
	if (NULL == p_line){
		return -1;
	}
	TP_LINE_OBJ *p_head = p_line->goto_node->hept_line;
	if (NULL == p_head){
		return 0;
	}
	p_line = p_head;
	int i = 0;
	do{
		if (i >= 20){
			return -1;
		}
		res[ i ] = p_line->id;
		count = ++i;
		p_line = p_line->nfne_line;
	}while(p_line != p_head);
	return count;
}

int cal_front_road_by_road(lua_State * L) {
	luaL_checktype(L,1,LUA_TNUMBER);
	uint64_t road_id = lua_tonumber(L,1);
	uint64_t road[20],i;
	
	int len_of_road = get_front_road_by_road(road_id,road);

	if (len_of_road<= 0 ) {
		lua_pushnil(L);
	}
	lua_newtable(L);
	for(i=0;i<len_of_road;i++) {
		lua_pushnumber(L,road[i]);
	//	printf("front id:%ld\t",road[i]);
		lua_rawseti(L,-2,i+1);
	}
	return 1;
	
	
}

int cal_poi_by_line(lua_State * L) {
    	int n;
	luaL_checktype(L,1,LUA_TTABLE);
	n= luaL_getn(L,1);  
	if (n==0) {
	    lua_pushnil(L);  
	    return 1;
	} 

	uint64_t line[n],i;
	for (i=1;i<=n;i++) {
	    	lua_rawgeti(L,1,i);
		line[i-1]=lua_tonumber(L,-1);
		//printf("line id :%ld\n",line[i-1]);
		lua_pop(L,1);
	}
	int len_of_line = n;
	//printf("len of line set :%d\n",n);
	uint64_t poi[100];
	int num_poi  = get_poi_by_line_set(line,len_of_line,poi);
	if (num_poi >100) {
		num_poi = 100;
	} 
	//printf("the num of poi :%d\n",num_poi);
	uint64_t res[num_poi][6];
	int res_num = get_poi_info_by_poi_set(poi,num_poi,res);

	lua_newtable(L);
	int j ;
	for (i=0;i<res_num;i++) {	
	    	//printf("test ---- result %ld,%ld,%ld,%ld,%ld,%ld\n",res[i][0],res[i][1],res[i][2],res[i][3],res[i][4],res[i][5]);
		for (j=0;j<6;j++) { 
		    lua_pushnumber(L,res[i][j]);
		    lua_rawseti(L,-2,(6*i+j)+1);	
		}
	}

	return 1;
}
