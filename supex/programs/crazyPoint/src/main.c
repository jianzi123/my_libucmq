#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "swift_api.h"
#include "swift_lua_api.h"
#include "crzpt_api.h"
#include "load_swift_cfg.h"
#include "load_crzpt_cfg.h"

#ifdef OPEN_CORO
#include "crzpt_coro_lua_api.h"
#else
#include "crzpt_line_lua_api.h"
#endif

static struct crzpt_cfg_temp_data g_temp_cfg_data = {};

struct swift_cfg_list g_swift_cfg_list = {};
struct crzpt_cfg_list g_crzpt_cfg_list[ MAX_APP_COUNTS ];

int main(int argc, char** argv)
{
	int i = 0;
	pid_t pid[ MAX_APP_COUNTS ] = {0};
	/*load swift config*/
	load_swift_cfg_argv(&g_swift_cfg_list.argv_info, argc, argv);
	load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);
	/*load crzpt config*/
	memset(g_crzpt_cfg_list, 0, sizeof(struct crzpt_cfg_list)*MAX_APP_COUNTS);
	load_crzpt_cfg_file(&g_temp_cfg_data, g_swift_cfg_list.argv_info.conf_name);
	for(i = 0; i < g_temp_cfg_data.app_counts; i++){
		memcpy( g_crzpt_cfg_list[i].argv_info.conf_name, 
				g_swift_cfg_list.argv_info.conf_name, MAX_FILE_NAME_SIZE );
		memcpy( g_crzpt_cfg_list[i].argv_info.serv_name,
				g_temp_cfg_data.app_names[i], MAX_FILE_NAME_SIZE );
		memcpy( g_crzpt_cfg_list[i].argv_info.msmq_name,
				g_temp_cfg_data.app_msmqs[i], MAX_FILE_NAME_SIZE );

		g_crzpt_cfg_list[i].file_info.roamer_counts = g_temp_cfg_data.roamer_counts;
#ifdef OPEN_CORO
		g_crzpt_cfg_list[i].file_info.pauper_counts = g_temp_cfg_data.pauper_counts;
#endif

		g_crzpt_cfg_list[i].file_info.log_path = x_strdup(g_swift_cfg_list.file_info.log_path);
		g_crzpt_cfg_list[i].file_info.log_file = x_strdup(g_swift_cfg_list.file_info.log_file);
		g_crzpt_cfg_list[i].file_info.log_level = g_swift_cfg_list.file_info.log_level;
	}
	/*config function*/
	for(i = 0; i < g_temp_cfg_data.app_counts; i++){
		pid[i] = fork();
		if (pid[i] < 0){
			x_perror("fork");
			exit(EXIT_FAILURE);
		}else if (pid[i] == 0){
			/*====crzpt====*/
			g_crzpt_cfg_list[i].vmsys_init = crzpt_vms_init;
			g_crzpt_cfg_list[i].vmsys_exit = crzpt_vms_exit;
			g_crzpt_cfg_list[i].vmsys_load = crzpt_vms_load;
			g_crzpt_cfg_list[i].vmsys_rfsh = crzpt_vms_rfsh;
			g_crzpt_cfg_list[i].vmsys_call = crzpt_vms_call;
			g_crzpt_cfg_list[i].vmsys_push = crzpt_vms_push;
			g_crzpt_cfg_list[i].vmsys_pull = crzpt_vms_pull;

			g_crzpt_cfg_list[i].store_firing = crzpt_sys_ldb_run;
			g_crzpt_cfg_list[i].store_insert = crzpt_sys_ldb_put;

			crzpt_mount(&g_crzpt_cfg_list[i]);
			crzpt_start();
		}else{
			printf("FORK ONE PROCESS -->PID :%d\n", pid[i]);
		}
	}
	/*====swift====*/
	g_swift_cfg_list.func_info[ APPLY_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[ APPLY_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_call;
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
