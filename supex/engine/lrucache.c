#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "ldb_cache.h"
#include "lrucache.h"

static leveldb_cache_t *g_lcache = NULL;

int lrucache_init(void) 
{
	//size_t cache_size = 20000;
	size_t cache_size = 1024*1024;
	size_t max_value_len = 512;
	g_lcache = leveldb_cache_create_lru(cache_size, max_value_len);
	return (g_lcache)? 0:-1;
}
int lrucache_destory(void)
{
	if(g_lcache){
		leveldb_cache_destroy(g_lcache);
	}
	return 0;
}


/*
---- 1 cache key (string)
---- 2 cache value
---- 3 cache value len
*/

int lrucache_setvalue(lua_State *L)
{
	if(3 != lua_gettop(L)){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[cache_setvalue]parameter must input cache_address, key, value");
		return 2;
	}

	if(!g_lcache){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "cache_setvalue cache_address is error");
		return 2;
	}

	const char *key = luaL_checkstring(L, 1);
	const char *val = luaL_checkstring(L, 2);
	size_t klen = strlen(key);
	size_t vlen = luaL_checkint(L, 3);
	if(klen == 0 || vlen == 0){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "cache_setvalue cache key or val length is error");
		return 2;
	}


	bool ok = leveldb_cache_insert(g_lcache, key, strlen(key), val, vlen );
	lua_pushboolean( L, ok );
	lua_pushstring(L, "");
	return 2;
}



int lrucache_getvalue(lua_State *L)
{
	if(1 != lua_gettop(L)){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[cache_getvalue]parameter must input cache_address, key");
		return 2;
	}

	if(!g_lcache){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[cache_getvalue]cache_setvalue cache_address is error");
		return 2;
	}

	const char *key = luaL_checkstring(L, 1);
	size_t klen = strlen(key);
	if(klen == 0){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[cache_getvalue]cache_setvalue cache key length is error");
		return 2;
	}

	void *handle = leveldb_cache_fix(g_lcache, key, klen);
	if(!handle){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[cache_getvalue]cache_setvalue leveldb_cache_fix failed");
		return 2;
	}
	// get value.
	size_t vlen = 0;
	char *val = (char *)leveldb_cache_value(g_lcache, handle, &vlen);

	lua_pushboolean( L, 1 );
	lua_pushlstring(L, val, vlen);
	
	// unfix entry.
	leveldb_cache_unfix(g_lcache, handle);

	return 2;
}


int lrucache_removevalue(lua_State *L)
{
	if(1 != lua_gettop(L)){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[cache_removevalue]parameter must input cache_address, key");
		return 2;
	}
	const char *key = luaL_checkstring(L, 1);
	size_t klen = strlen(key);
	if(g_lcache && klen > 0){
		leveldb_cache_erase(g_lcache, key, klen);
	}

	lua_pushboolean( L, 1 );
	lua_pushstring(L, "");
	return 2;
}
