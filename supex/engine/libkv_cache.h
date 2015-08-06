#pragma once
#include <lauxlib.h>
#include <lualib.h>
/*
 * 函 数:libkv_init
 * 功 能:初始化libkv库
 * 参 数:
 * 功 能:
 * 返回值:
 */
void libkv_init();
/*
 * 函 数:libkv_setvalue
 * 功 能:设置值
 * 参 数:
 * 返回值：
 * 说明
 */
int libkv_setvalue(lua_State *L);
/*
 * 函 数:libkv_getvalue
 * 功 能:获取值
 * 参 数:
 * 返回值:
 * 说 明:
 */
int libkv_getvalue(lua_State *L);

/*
 * 函 数:libkv_deletekey
 * 功 能:删除key
 */
int libkv_deletekey(lua_State* L);