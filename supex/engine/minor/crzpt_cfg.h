#pragma once

#include "utils.h"

struct crzpt_cfg_argv {
	char conf_name[ MAX_FILE_NAME_SIZE ];
	char serv_name[ MAX_FILE_NAME_SIZE ];
	char msmq_name[ MAX_FILE_NAME_SIZE ];
};

struct crzpt_cfg_file {
	short roamer_counts;
#ifdef OPEN_CORO
	short pauper_counts;
#endif
        
	char *log_path;
        char *log_file;
        short log_level;
};

struct crzpt_cfg_list {
	struct crzpt_cfg_argv argv_info;
	struct crzpt_cfg_file file_info;

	void (*entry_init)(void);

	bool (*task_lookup)(void *user, void *task);
	bool (*task_report)(void *user, void *task);

#ifdef OPEN_CORO
	int (*vmsys_init)(void *user, void *task, int step);
	int (*vmsys_exit)(void *user, void *task, int step);
	int (*vmsys_load)(void *user, void *task, int step);
	int (*vmsys_rfsh)(void *user, void *task, int step);
	int (*vmsys_call)(void *user, void *task, int step);
	int (*vmsys_push)(void *user, void *task, int step);
	int (*vmsys_pull)(void *user, void *task, int step);
#else
	int (*vmsys_init)(void *user, void *task);
	int (*vmsys_exit)(void *user, void *task);
	int (*vmsys_load)(void *user, void *task);
	int (*vmsys_rfsh)(void *user, void *task);
	int (*vmsys_call)(void *user, void *task);
	int (*vmsys_push)(void *user, void *task);
	int (*vmsys_pull)(void *user, void *task);
#endif
	void (*store_firing)(const char *name, size_t block_size, size_t wb_size, size_t lru_size, short bloom_size);
	int (*store_insert)(const char *key, size_t klen, const char *value, size_t vlen);
};
