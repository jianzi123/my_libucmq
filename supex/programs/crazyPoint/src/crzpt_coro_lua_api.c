#include <assert.h>

#include "match.h"
#include "crzpt_api.h"
#include "crzpt_plan.h"
#include "crzpt_coro_lua_api.h"

extern int async_http(lua_State *L);

static lua_State *_vms_new( void )
{
	int error = 0;
	lua_State *L = luaL_newstate();
	assert( L );
	luaopen_base( L );
	luaL_openlibs( L );
	/*reg func*/
	lua_register(L, "crzpt_http", async_http);
	/*lua init*/
	{
		extern struct crzpt_cfg_list g_crzpt_cfg_list;

		int app_lua_get_serv_name(lua_State *L){
			lua_pushstring( L, g_crzpt_cfg_list.argv_info.serv_name );
			return 1;
		}
		lua_register(L, "app_lua_get_serv_name", app_lua_get_serv_name);

		error = luaL_dofile(L, "crzpt_lua/core/init.lua");
		if (error) {
			fprintf(stderr,"%s\n", lua_tostring(L,-1));
			lua_pop(L,1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	lua_register(L, "app_lua_make_plan", crzpt_lua_make_plan);
	error = luaL_dofile(L, "crzpt_lua/core/coro_start.lua");
	if (error) {
		fprintf(stderr,"%s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		exit(EXIT_FAILURE);
	}
	return L;
}

static int _vms_init( lua_State **L, int last, struct crzpt_task_node *task, long S )
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
int crzpt_vms_init( void *user, void *task, int step )
{
	int error = 0;
	error = crzpt_for_batch_vm( user, task, step, _vms_init );
	if (error) {
		exit(EXIT_FAILURE);
	}
	return error;
}

static int _vms_exit( lua_State **L, int last, struct crzpt_task_node *task, long S )
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
int crzpt_vms_exit( void *user, void *task, int step )
{
	int error = crzpt_for_batch_vm( user, task, step, _vms_exit );
	x_printf(S, "exit one batch LUA!\n");
	return error;
}


static int _vms_load( lua_State **L, int last, struct crzpt_task_node *task, long S )
{
	lua_getglobal( *L, "app_load" );
	return lua_pcall( *L, 0, 0, 0 );
}
int crzpt_vms_load( void *user, void *task, int step )
{
	return crzpt_for_alone_vm( user, task, step, _vms_load );
}


static int _vms_rfsh( lua_State **L, int last, struct crzpt_task_node *task, long S )
{
	lua_getglobal( *L, "app_rfsh" );
	return lua_pcall( *L, 0, 0, 0 ) ;
}
int crzpt_vms_rfsh( void *user, void *task, int step )
{
	return crzpt_for_batch_vm( user, task, step, _vms_rfsh );
}


static int _vms_call( lua_State **L, int last, struct crzpt_task_node *task, long S )
{
	struct crzpt_plan_node *p_plan = (struct crzpt_plan_node *)task->data;

	assert(p_plan);
	lua_getglobal( *L, "app_call" );
	lua_pushboolean( *L, last );
	lua_pushinteger( *L, S );
	lua_pushinteger( *L, p_plan->pidx );
	return lua_pcall( *L, 3, 0, 0 );
}
int crzpt_vms_call( void *user, void *task, int step )
{
	return crzpt_for_alone_vm( user, task, step, _vms_call );
}

static int _vms_push( lua_State **L, int last, struct crzpt_task_node *task, long S )
{
	struct msg_info *p_msg = (struct msg_info *)task->data;

	assert(p_msg);
	lua_getglobal( *L, "app_push" );
	lua_pushboolean( *L, last );
	lua_pushinteger( *L, S );
	lua_pushstring( *L, p_msg->data );
	return lua_pcall( *L, 3, 0, 0 );
}
int crzpt_vms_push( void *user, void *task, int step )
{
	return crzpt_for_batch_vm( user, task, step, _vms_push );
}


static int _vms_pull( lua_State **L, int last, struct crzpt_task_node *task, long S )
{
	struct msg_info *p_msg = (struct msg_info *)task->data;

	lua_getglobal( *L, "app_pull" );
	lua_pushboolean( *L, last );
	lua_pushinteger( *L, S );
	lua_pushstring( *L, p_msg->data );
	return lua_pcall( *L, 3, 0, 0 );
}
int crzpt_vms_pull( void *user, void *task, int step )
{
	return crzpt_for_alone_vm( user, task, step, _vms_pull );
}
