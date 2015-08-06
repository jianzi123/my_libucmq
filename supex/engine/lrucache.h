#pragma once
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


int lrucache_init(void);
int lrucache_destory(void);
int lrucache_setvalue(lua_State *L);
int lrucache_getvalue(lua_State *L);
int lrucache_removevalue(lua_State *L);
