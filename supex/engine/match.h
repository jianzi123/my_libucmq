#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


extern int app_lua_mapinit(lua_State *L);
extern int app_lua_convert(lua_State *L);
extern int app_lua_reverse(lua_State *L);
extern int app_lua_ifmatch(lua_State *L);
