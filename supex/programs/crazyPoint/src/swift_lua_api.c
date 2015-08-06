#include <assert.h>

#include "match.h"
#include "swift_api.h"
#include "swift_lua_api.h"

extern int sync_http(lua_State *L);
extern int app_lua_get_head_data(lua_State *L);
extern int app_lua_get_body_data(lua_State *L);
extern int app_lua_get_path_data(lua_State *L);
extern int app_lua_get_uri_args(lua_State *L);
extern int app_lua_add_send_data(lua_State *L);

int app_lua_diffuse(lua_State *L)
{
	size_t size = 0;
	char *name = (char *)lua_tostring(L, 1);
	char *data = (char *)lua_tolstring(L, 2, &size);
	int time = (int)lua_tointeger(L, 3);
	int mode = (int)lua_tointeger(L, 4);
	printf("time is %d mode is %d #################\n", time, mode);
	
	struct msg_info msg = {};
	msg.mode = (char)mode;
	msg.time = time;
	memcpy(msg.data, data, MIN(size, MSMQ_MAX_DATA_SIZE));
	int ok = msmq_send(name, (char *)&msg, sizeof(struct msg_info));
	if (ok != 0){
		lua_pushboolean( L, 0 );
	}else{
		lua_pushboolean( L, 1 );
	}
	return 1;
}


	
static int _vms_cntl( lua_State **L, int last, struct swift_task_node *task )
{
	struct msg_info *msg = task->data;
	assert( msg );

	lua_getglobal( *L, "app_cntl" );
	lua_pushboolean( *L, last );
	lua_pushstring( *L, msg->data );
	switch (msg->opt){
		case 'l':
			lua_pushstring(*L, "insmod");
			break;
		case 'f':
			lua_pushstring(*L, "rmmod");
			break;
		case 'o':
			lua_pushstring(*L, "open");
			break;
		case 'c':
			lua_pushstring(*L, "close");
			break;
		case 'd':
			lua_pushstring(*L, "delete");
			break;
		default:
			x_printf(S, "Error msmq opt!\n");
			return 0;
	}
	return lua_pcall( *L, 3, 0, 0 ) ;
}
int swift_vms_cntl( void *W )
{
	return swift_for_alone_vm( W, _vms_cntl );
}


static lua_State *_vms_new( void )
{
	int error = 0;
	lua_State *L = luaL_newstate();
	assert( L );
	luaopen_base( L );
	luaL_openlibs( L );
	/*reg func*/
	lua_register(L, "supex_http", sync_http);
	lua_register(L, "app_lua_diffuse", app_lua_diffuse);
	lua_register(L, "app_lua_get_head_data", app_lua_get_head_data);
	lua_register(L, "app_lua_get_body_data", app_lua_get_body_data);
	lua_register(L, "app_lua_get_path_data", app_lua_get_path_data);
	lua_register(L, "app_lua_get_uri_args", app_lua_get_uri_args);
	lua_register(L, "app_lua_add_send_data", app_lua_add_send_data);
	lua_register(L, "app_lua_mapinit", app_lua_mapinit);
	lua_register(L, "app_lua_convert", app_lua_convert);
	lua_register(L, "app_lua_reverse", app_lua_reverse);
	lua_register(L, "app_lua_ifmatch", app_lua_ifmatch);
	/*lua init*/
	{
		extern struct swift_cfg_list g_swift_cfg_list;

		int app_lua_get_serv_name(lua_State *L){
			lua_pushstring( L, g_swift_cfg_list.argv_info.serv_name );
			return 1;
		}
		lua_register(L, "app_lua_get_serv_name", app_lua_get_serv_name);

		error = luaL_dofile(L, "swift_lua/core/init.lua");
		if (error) {
			fprintf(stderr,"%s\n", lua_tostring(L,-1));
			lua_pop(L,1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "swift_lua/core/line_start.lua");
	if (error) {
		fprintf(stderr,"%s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		exit(EXIT_FAILURE);
	}
	return L;
}
static int _vms_init( lua_State **L, int last, struct swift_task_node *task )
{
	if (*L != NULL){
		x_printf(S, "No need to init LUA VM!\n");
		return 0;
	}
	*L = _vms_new( );
	assert( *L );
	lua_getglobal(*L, "app_init");
	return lua_pcall(*L, 0, 0, 0);
}
int swift_vms_init( void *W )
{
	int error = 0;
	error = swift_for_alone_vm( W, _vms_init );
	if (error) {
		exit(EXIT_FAILURE);
	}
	return error;
}




static int _vms_exit( lua_State **L, int last, struct swift_task_node *task )
{
	int error = 0;

	lua_getglobal(*L, "app_exit");
	error = lua_pcall(*L, 0, 0, 0);
	if (error) {
		x_printf(E, "%s\n", lua_tostring( *L, -1 ));
		lua_pop( *L, 1 );
	}
	lua_close( *L );
	*L = NULL;
	return 0;/*must return 0*/
}

int swift_vms_exit( void *W )
{
	int error = swift_for_alone_vm( W, _vms_exit );
	x_printf(S, "exit one alone LUA!\n");
	return error;
}

static int _vms_rfsh( lua_State **L, int last, struct swift_task_node *task )
{
	lua_getglobal( *L, "app_rfsh" );
	lua_pushboolean( *L, last );
	lua_pushinteger( *L, task->sfd );
	return lua_pcall( *L, 2, 0, 0 ) ;
}
int swift_vms_rfsh( void *W )
{
	return swift_for_alone_vm( W, _vms_rfsh );
}


static int _vms_sync( lua_State **L, int last, struct swift_task_node *task )
{
	lua_getglobal( *L, "app_push" );
	lua_pushboolean( *L, last );
	lua_pushinteger( *L, task->sfd );
	return lua_pcall( *L, 2, 0, 0 ) ;
}
int swift_vms_sync( void *W )
{
	return swift_for_alone_vm( W, _vms_sync );
}


static int _vms_gain( lua_State **L, int last, struct swift_task_node *task )
{
	lua_getglobal( *L, "app_pull" );
	lua_pushboolean( *L, last );
	lua_pushinteger( *L, task->sfd );
	return lua_pcall( *L, 2, 0, 0 ) ;
}
int swift_vms_gain( void *W )
{
	return swift_for_alone_vm( W, _vms_gain );
}



static int _vms_call( lua_State **L, int last, struct swift_task_node *task )
{
	lua_getglobal( *L, "app_call_all" );
	lua_pushboolean( *L, last );
	lua_pushinteger( *L, task->sfd );
	return lua_pcall( *L, 2, 0, 0 ) ;
}
int swift_vms_call( void *W )
{
	return swift_for_alone_vm( W, _vms_call );
}


static int _vms_exec( lua_State **L, int last, struct swift_task_node *task )
{
	lua_getglobal( *L, "app_call_one" );
	lua_pushboolean( *L, last );
	lua_pushinteger( *L, task->sfd );
	return lua_pcall( *L, 2, 0, 0 ) ;
}
int swift_vms_exec( void *W )
{
	return swift_for_alone_vm( W, _vms_exec );
}
