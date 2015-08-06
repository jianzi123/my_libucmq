#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "load_cfg.h"
#include "smart_api.h"

#ifdef OPEN_CORO
#include "smart_coro_lua_api.h"
#else
#include "smart_line_lua_api.h"
#endif

struct smart_cfg_list g_smart_cfg_list = {};

int main(int argc, char** argv)
{
	load_cfg_argv(&g_smart_cfg_list.argv_info, argc, argv);

	load_cfg_file(&g_smart_cfg_list.file_info, g_smart_cfg_list.argv_info.conf_name);

	g_smart_cfg_list.func_info[ APPLY_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[ APPLY_FUNC_ORDER ].func = (SUPEX_TASK_CALLBACK)smart_vms_call;
	g_smart_cfg_list.func_info[ FETCH_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[ FETCH_FUNC_ORDER ].func = (SUPEX_TASK_CALLBACK)smart_vms_gain;
	g_smart_cfg_list.func_info[ MERGE_FUNC_ORDER ].type = BIT8_TASK_TYPE_WHOLE;
	g_smart_cfg_list.func_info[ MERGE_FUNC_ORDER ].func = (SUPEX_TASK_CALLBACK)smart_vms_sync;
	g_smart_cfg_list.func_info[ CUSTOM_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[ CUSTOM_FUNC_ORDER ].func = (SUPEX_TASK_CALLBACK)smart_vms_exec;

	g_smart_cfg_list.entry_init = NULL;
	
	g_smart_cfg_list.vmsys_init = smart_vms_init;
	g_smart_cfg_list.vmsys_exit = smart_vms_exit;
	g_smart_cfg_list.vmsys_cntl = smart_vms_cntl;
	g_smart_cfg_list.vmsys_rfsh = smart_vms_rfsh;

	smart_mount(&g_smart_cfg_list);
	smart_start();
	return 0;
}
