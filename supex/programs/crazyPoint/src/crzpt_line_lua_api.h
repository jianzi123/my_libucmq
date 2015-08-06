#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

void crzpt_sys_ldb_run(const char *name, size_t block_size, size_t wb_size, size_t lru_size, short bloom_size);

int crzpt_sys_ldb_put(const char *key, size_t klen, const char *value, size_t vlen);


int crzpt_vms_init(void *user, void *task);

int crzpt_vms_exit(void *user, void *task);

int crzpt_vms_load(void *user, void *task);

int crzpt_vms_rfsh(void *user, void *task);

int crzpt_vms_call(void *user, void *task);

int crzpt_vms_push(void *user, void *task);

int crzpt_vms_pull(void *user, void *task);
