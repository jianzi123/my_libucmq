#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "load_swift_cfg.h"
#include "swift_api.h"

#include "swift_lua_api.h"

struct swift_cfg_list g_swift_cfg_list = {};

int main(int argc, char** argv)
{
	load_cfg_argv(&g_swift_cfg_list.argv_info, argc, argv);

	load_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	g_swift_cfg_list.func_info[ APPLY_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[ APPLY_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_call;
	g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_gain;
	g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].type = BIT8_TASK_TYPE_WHOLE;
	g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_sync;
	g_swift_cfg_list.func_info[ CUSTOM_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[ CUSTOM_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_exec;

	g_swift_cfg_list.vmsys_init = swift_vms_init;
	g_swift_cfg_list.vmsys_exit = swift_vms_exit;
	g_swift_cfg_list.vmsys_cntl = swift_vms_cntl;
	g_swift_cfg_list.vmsys_rfsh = swift_vms_rfsh;

	swift_mount(&g_swift_cfg_list);
	swift_start();
	return 0;
}
