#pragma once

#include "utils.h"
#include "crzpt_cfg.h"

#define MAX_APP_COUNTS	100

struct crzpt_cfg_temp_data {
	short roamer_counts;
#ifdef OPEN_CORO
	short pauper_counts;
#endif

	short app_counts;
	char app_names[ MAX_APP_COUNTS ][ MAX_FILE_NAME_SIZE ];
	char app_msmqs[ MAX_APP_COUNTS ][ MAX_FILE_NAME_SIZE ];
};

void load_crzpt_cfg_file(struct crzpt_cfg_temp_data *p_cfg, char *name);
