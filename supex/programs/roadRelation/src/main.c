#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "load_cfg.h"
#include "smart_api.h"
#include "entry.h"

#ifdef OPEN_CORO
#include "smart_coro_cpp_api.h"
#else
#include "smart_line_cpp_api.h"
#endif

static struct smart_cfg_list g_cfg_list = {};

int main(int argc, char** argv)
{
	load_cfg_argv(&g_cfg_list.argv_info, argc, argv);

	load_cfg_file(&g_cfg_list.file_info, g_cfg_list.argv_info.conf_name);

	g_cfg_list.func_info[ HMGET_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_cfg_list.func_info[ HMGET_FUNC_ORDER ].func = (SUPEX_TASK_CALLBACK)api_hmget;
	g_cfg_list.func_info[ HGETALL_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_cfg_list.func_info[ HGETALL_FUNC_ORDER ].func = (SUPEX_TASK_CALLBACK)api_hgetall;
	g_cfg_list.entry_init = entry_init;

	smart_mount(&g_cfg_list);
	smart_start();
	return 0;
}
