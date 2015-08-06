/*
 * auth: baoxue
 * date: Sat Aug  3 10:20:26 CST 2013
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
#include "tcp_api.h"
#include "async_link.h"



int async_http(lua_State *L) {
	/* check these parameters */
	size_t size = luaL_checkint(L, 5);
	const char *data = luaL_checkstring(L, 4);
	short port = (short)luaL_checkint(L, 3);
	const char *host = luaL_checkstring(L, 2);
	struct schedule *S = (struct schedule *)luaL_checkinteger(L, 1);

	char *back = NULL;
	x_printf(D, "L backuse is %p\n", L);
	int ok = async_tcp_ask(host, port, data, size, &back, 0, S);
	if (ok > 0){
		lua_pushboolean(L, 1);
		lua_pushlstring(L, back, ok);
		free(back);
	}else{
		lua_pushboolean(L, 0);
		switch (ok) {
			case TCP_ERR_CONNECT:
				lua_pushfstring(L, "connect %s:%d failed!", host, port);
				break;
			case TCP_ERR_SEND:
				lua_pushfstring(L, "send fail!");
				break;
			case TCP_ERR_SOCKOPT:
				lua_pushstring(L, "can't set recv timeout!");
				break;
			case TCP_ERR_MEMORY:
				lua_pushstring(L, "no more memory!");
				break;
			case TCP_ERR_TIMEOUT:
				lua_pushstring(L, "time out!");
				break;
			default:
				lua_pushstring(L, "unknow error!");
				break;
		}
	}
	return 2;
}
