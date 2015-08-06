#pragma once

#include "utils.h"
#include "major_def.h"

struct smart_cfg_argv {
	char conf_name[ MAX_FILE_NAME_SIZE ];
	char serv_name[ MAX_FILE_NAME_SIZE ];
	char msmq_name[ MAX_FILE_NAME_SIZE ];
};

struct smart_cfg_file {
        short srv_port;
	short hander_counts;
	short worker_counts;
        short monitor_times;    //监控定时器时长
#ifdef OPEN_CORO
	short tasker_counts;
#endif
        size_t max_req_size;
        
	char *log_path;
        char *log_file;
        short log_level;

#ifdef USE_HTTP_PROTOCOL
	short api_counts;
	char *api_apply;
	char *api_fetch;
	char *api_merge;
	char api_names[ MAX_API_COUNTS ][ MAX_API_NAME_LEN + 1 ];
#endif
};
struct smart_cfg_func {
	char type;
	SUPEX_TASK_CALLBACK func;
};

struct smart_cfg_list {
	struct smart_cfg_argv argv_info;
	struct smart_cfg_file file_info;
	struct smart_cfg_func func_info[ LIMIT_FUNC_ORDER ];

	void (*entry_init)(void);
	
	bool (*task_lookup)(void *user, void *task);
	bool (*task_report)(void *user, void *task);

#ifdef OPEN_CORO
	int (*vmsys_init)(void *user, void *task, int step);
	int (*vmsys_exit)(void *user, void *task, int step);
	int (*vmsys_cntl)(void *user, void *task, int step);
	int (*vmsys_rfsh)(void *user, void *task, int step);
        int (*vmsys_monitor)(void* user, void* task, int step);
#else
	int (*vmsys_init)(void *user, void *task);
	int (*vmsys_exit)(void *user, void *task);
	int (*vmsys_cntl)(void *user, void *task);
	int (*vmsys_rfsh)(void *user, void *task);
        int (*vmsys_monitor)(void* user, void* task);
#endif
};
