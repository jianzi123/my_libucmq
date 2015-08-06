#include "libkv_cache.h"
#include "libkv.h"
#include <assert.h>


/*
 * 函 数:libkv_init
 * 功 能:初始化libkv库
 * 参 数:
 * 功 能:
 * 返回值:
 */
void libkv_init()
{
    assert(kv_init(NULL) == ERR_NONE);
}


int libkv_setvalue(lua_State *L)
{
    if(2 != lua_gettop(L)){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[libkv_setvalue]parameter must input command, len of command");
		return 2;
	}
    const char *command = luaL_checkstring(L, 1);
    long len = luaL_checklong(L, 2);
    if(NULL == command || 0 == len){
        lua_pushboolean( L, 0 );
	lua_pushstring(L, "cache_setvalue cache key or val length is error");
	return 2;
    }
    answer_t *ans = NULL;
    ans = kv_ask(command, len);
    if(ERR_NONE != ans->errnum){
        lua_pushboolean(L, 0);
        lua_pushstring(L, ans->err);
	answer_release(ans);
        return 2;
    }
    answer_release(ans);
    lua_pushboolean( L, 1 );
    lua_pushstring(L, "");
    return 2;
}

int libkv_getvalue(lua_State *L)
{
    if(2 != lua_gettop(L)){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[libkv_getvalue]parameter must input command , len of command");
		return 2;
	}

	const char *command = luaL_checkstring(L, 1);
	long len = luaL_checklong(L,2);
	if(NULL == command || 0 == len){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[libkv__getvalue] command or len of command is error");
		return 2;
	}

	answer_t*ans = NULL;
	ans = kv_ask(command, len);
	if(ERR_NONE != ans->errnum){
		lua_pushboolean(L,0);
		lua_pushstring(L,ans->err);
		answer_release(ans);
		return 2;
	}
	answer_value_t* value = NULL;		
	value = answer_first_value(ans);
	if(NULL != value){
		lua_pushboolean(L,1);
		lua_pushstring(L,answer_value_to_string(value));
		answer_release(ans);
		return 2;
	}

	answer_release(ans);
	lua_pushboolean( L, 0 );
	lua_pushstring(L, "[libkv_getvalue] get failed");
	return 2;
}

int libkv_deletekey(lua_State* L)
{
    if(2 != lua_gettop(L)){
		lua_pushboolean( L, 0 );
		lua_pushstring(L, "[libkv_deletekey]parameter must input command , len of command");
		return 2;
	}
    const char *command = luaL_checkstring(L, 1);
    long len = luaL_checklong(L,2);
    if(NULL == command || 0 == len){
            lua_pushboolean( L, 0 );
            lua_pushstring(L, "[libkv__getvalue] command or len of command is error");
            return 2;
        }
    answer_t*ans = NULL;
    ans = kv_ask(command, len);
    if(ERR_NONE != ans->errnum){
            lua_pushboolean(L,0);
            lua_pushstring(L,ans->err);
	    answer_release(ans);
            return 2;
	}
    answer_release(ans);
    lua_pushboolean( L, 1 );
    lua_pushstring(L, "");
    return 2;
}
