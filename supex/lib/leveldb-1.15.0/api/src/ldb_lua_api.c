#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ldb_lua_api.h"
#include "ldb.h"

extern struct _leveldb_stuff *g_supex_ldbs;

int ldb_lua_set(lua_State *L)
{
    const char *key = lua_tostring(L, 1);
    const char *val = lua_tostring(L, 2);

    int ret = ldb_put(g_supex_ldbs, key, strlen(key), val, strlen(val));
    if (ret != 0) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, 1);
    return 1;
}

int ldb_lua_get(lua_State *L)
{
    char *val;
    int vlen;

    const char *key = lua_tostring(L, 1);

    val = ldb_get(g_supex_ldbs, key, strlen(key), &vlen);
    if (val == NULL) {
        lua_pushboolean(L, 0);
        return 0;
    }

    lua_pushboolean(L, 1);
    if (vlen == 0) {
        lua_pushboolean(L, 0);
        return 2;
    }

    lua_pushlstring(L, val, vlen);
    free(val);
    return 2;    
}

int ldb_lua_del(lua_State *L)
{
    const char *key = lua_tostring(L, 1);

    int ret = ldb_delete(g_supex_ldbs, key, strlen(key));
    if (ret == -1) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, 1);
    return 1;
}
