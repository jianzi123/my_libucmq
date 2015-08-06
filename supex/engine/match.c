#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "match.h"


static int g_map_index = 0;
static int g_map_least = 0;
static int g_map_excess = 0;
static int g_map_size = 0;
//LOCK

#define MAP_BIT_ON_OFF( bit , offset )		( ((bit) == '0')? 0 : (1 << (offset)) )

int app_lua_mapinit(lua_State *L)
{
	int len = lua_tonumber(L, 1);
	if (len <= 0){
		lua_pushnil(L);
		return 1;//FIXME to check
	}
	if (g_map_index == 0){
		g_map_index =  len / (sizeof(long) * 8);
	}
	if (g_map_least == 0){
		g_map_least =  len % (sizeof(long) * 8);
	}
	if (g_map_excess == 0){
		g_map_excess = ( g_map_index ) * sizeof(long);
	}
	if (g_map_size == 0){
		g_map_size = ( g_map_index + ((g_map_least >= 1)? 1 : 0) ) * sizeof(long);
	}
	char *result = malloc( g_map_size );
	lua_pushinteger(L, (long)result);/* [int] can't run on bit 64 computer */
	return 1;
}
#if 1
static void compress(char *dst, const char *src)
{
	int i,step = 0;

	memset(dst, 0, g_map_size);

	for(i=0; i < g_map_excess; i++){
		dst[ i ] = 0 | ((src[0]-'0') << 0) | ((src[1]-'0') << 1) | ((src[2]-'0') << 2) | ((src[3]-'0') << 3) 
			| ((src[4]-'0') << 4) | ((src[5]-'0') << 5) | ((src[6]-'0') << 6) | ((src[7]-'0') << 7);
		/*
		dst[ i ] = 0 | MAP_BIT_ON_OFF(src[0], 0) | MAP_BIT_ON_OFF(src[1], 1) | MAP_BIT_ON_OFF(src[2], 2) | MAP_BIT_ON_OFF(src[3], 3) 
			| MAP_BIT_ON_OFF(src[4], 4) | MAP_BIT_ON_OFF(src[5], 5) | MAP_BIT_ON_OFF(src[6], 6) | MAP_BIT_ON_OFF(src[7], 7);
		*/
		src += 8;
	}
	for(i=0; i< g_map_least; i++){
		step = i / 8;
		dst[ g_map_excess + step] |= ( (src[i]-'0') << (i % 8) );
		//dst[ g_map_excess + step] |= MAP_BIT_ON_OFF(src[i], i % 8);
	}
	return;
}
int app_lua_convert(lua_State *L)
{
	char *p_dst = NULL;
	const char *p_src = NULL;
	p_dst = (char *)lua_tointeger(L, 1);
	p_src = lua_tostring(L, 2);
	if ((p_dst == NULL) || (p_src == NULL) ){
		lua_pushnil(L);
		return 1;//FIXME to check
	}
	compress(p_dst, p_src);
	lua_pushlstring( L, (const char *)p_dst, g_map_size );
	return 1;
}
int app_lua_reverse(lua_State *L)
{
	int i,idx = 0;
	long *p_dst = NULL;
	const char *p_src = NULL;
	p_dst = (long *)lua_tointeger(L, 1);
	p_src = lua_tostring(L, 2);
	if ((p_dst == NULL) || (p_src == NULL) ){
		lua_pushnil(L);
		return 1;//FIXME to check
	}
	compress((char *)p_dst, p_src);
	idx = g_map_size / sizeof(long);
	for (i=0; i < idx; i++){
		p_dst[i] = ~p_dst[i];
	}
	lua_pushlstring( L, (const char *)p_dst, g_map_size );
	return 1;
}
#else
static void compress(char *dst, lua_State *L)
{
	int i,step,val = 0;

	memset(dst, 0, g_map_size);

	int size = lua_objlen(L, -1);
	for(i=0; i < size; i++){
		lua_pushnumber(L, i + 1);
		lua_rawget(L, -2);
		val = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);

		step = i / 8;
		dst[ step ] |= ( val << (i % 8) );
	}
	return;
}
int app_lua_convert(lua_State *L)
{
	char *p_dst = NULL;
	p_dst = (char *)lua_tointeger(L, 1);
	if (p_dst == NULL){
		lua_pushnil(L);
		return 1;//FIXME to check
	}
	compress(p_dst, L);
	lua_pushlstring( L, (const char *)p_dst, g_map_size );
	return 1;
}
int app_lua_reverse(lua_State *L)
{
	int i,idx = 0;
	long *p_dst = NULL;
	p_dst = (long *)lua_tointeger(L, 1);
	if (p_dst == NULL){
		lua_pushnil(L);
		return 1;//FIXME to check
	}
	compress((char *)p_dst, L);
	idx = g_map_size / sizeof(long);
	for (i=0; i < idx; i++){
		p_dst[i] = ~p_dst[i];
	}
	lua_pushlstring( L, (const char *)p_dst, g_map_size );
	return 1;
}
#endif

int app_lua_ifmatch(lua_State *L)
{
	int i,idx = 0;
	size_t dlen,slen = 0;
	long *p_dst = NULL;
	long *p_src = NULL;
	p_dst = (long *)lua_tolstring (L, 1, &dlen);
	p_src = (long *)lua_tolstring (L, 2, &slen);
	if ((p_dst == NULL) || (p_src == NULL)){
		lua_pushboolean(L, 0);
		return 1;
	}
	idx = ((dlen <= slen) ? dlen : slen) / sizeof(long);
	for (i=0;i<idx;i++){
		if( (p_dst[i] & p_src[i]) != 0 ){
			lua_pushboolean(L, 0);
			return 1;
		}
	}
	lua_pushboolean(L, 1);
	return 1;
}

