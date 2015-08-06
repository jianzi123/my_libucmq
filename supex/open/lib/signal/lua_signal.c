#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <sys/types.h>
#include <unistd.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

typedef void (*SIGN_HANDLER_FUN)(int);

static const char *lua_func_cmds = NULL;
/*<------------------------------------------------------------------------>*/
static int print(lua_State *L)
{
	printf(lua_tostring (L, 1));
	return 0;
}

static int send(lua_State *L)
{
	int pid = lua_tointeger (L, 1); 
	kill(pid, SIGUSR1);
	return 0;
}

static int get_pid(lua_State *L)
{
	int pid = getpid();
	lua_pushinteger(L, pid);
	return 1;
}
static int install(lua_State *L)
{
	int type= lua_type(L, 1);
	SIGN_HANDLER_FUN domain = NULL;
	printf("args type : ");
	switch (type){
		case LUA_TNIL:
			printf("1\n");
			break;
		case LUA_TNUMBER:
			printf("2\n");
			break;
		case LUA_TBOOLEAN:
			printf("3\n");
			break;
		case LUA_TSTRING:
			printf("4\n");
			lua_func_cmds = lua_tostring (L, 1);
			break;
		case LUA_TTABLE:
			printf("5\n");
			break;
		case LUA_TFUNCTION:
			printf("6\n");
			lua_CFunction sig_handler = lua_tocfunction (L, 1);
			lua_cpcall(L, sig_handler, NULL);
			break;
		case LUA_TUSERDATA:
			printf("7\n");
			break;
		case LUA_TTHREAD:
			printf("8\n");
			break;
		case LUA_TLIGHTUSERDATA:
			printf("9\n");
			break;
		default:
			printf("10\n");
			break;
	
	
	}
	//int test=0;
	{
		void run_lua_string(int signum){
			//test++;
			//printf("--------sign %d-------\n", signum);
			lua_State *L = lua_open();
			luaopen_base(L);
			luaL_openlibs(L);
			if(luaL_dostring(L, lua_func_cmds)){
				const char* errMsg = lua_tostring(L, -1);
				printf("%s\n", errMsg);
			};
			//luaL_dostring(L, "print('xxxxxxxx')");
			lua_close(L);
			//return test;
		}
		//test = run_lua_string();
		//lua_pushnumber(L, test);
		domain = run_lua_string;
	}
#if 0
	domain(1);
	return 0;

#else
	//sigset_t block;
	//struct sigaction action, old_action;
	struct sigaction action;

	action.sa_handler = (void *)domain;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	//sigaction(SIGINT, NULL, &old_action);
	//if (old_action.sa_handler != SIG_IGN) {
		//sigaction(SIGINT, &action, NULL);
		//sigaction(SIGRTMIN, &action, NULL);
		sigaction(SIGUSR1, &action, NULL);
	//}

	//sigemptyset(&block);
	//sigaddset(&block, SIGINT);
	//printf("block SIGINT\n");
	//sigprocmask(SIG_BLOCK, &block, NULL);
	//printf("unblock SIGINT\n");
	//sigprocmask(SIG_UNBLOCK, &block, NULL);
	return 0;
#endif
}

static const struct luaL_Reg lib[] =
{
	{"install", install},
	{"send", send},
	{"get_pid", get_pid},
	{"print", print},
	{NULL, NULL}
};

int luaopen_luasignal(lua_State *L) {
	luaL_register(L, "luasignal", lib);
	return 1;
}
