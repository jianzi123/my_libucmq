#pragma once

#include "utils.h"
#include "major_def.h"

struct swift_cfg_argv {
	char conf_name[ MAX_FILE_NAME_SIZE ];
	char serv_name[ MAX_FILE_NAME_SIZE ];
	char msmq_name[ MAX_FILE_NAME_SIZE ];
};

struct swift_cfg_file {
	short srv_port;
	short porter_counts;

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
struct swift_cfg_func {
	char type;
	TASK_CALLBACK func;
};

struct swift_cfg_list {
	struct swift_cfg_argv argv_info;
	struct swift_cfg_file file_info;
	struct swift_cfg_func func_info[ LIMIT_FUNC_ORDER ];

	void (*entry_init)(void);
	void (*pthrd_init)(void *user);

	int (*vmsys_init)(void *W);
	int (*vmsys_exit)(void *W);
	int (*vmsys_cntl)(void *W);
	int (*vmsys_rfsh)(void *W);
	int (*vmsys_idle)(void *W);
};
