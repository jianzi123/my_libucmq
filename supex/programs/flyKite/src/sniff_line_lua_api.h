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



int sniff_vms_init(void *user, void *task);

int sniff_vms_exit(void *user, void *task);

int sniff_vms_cntl(void *user, void *task);

int sniff_vms_rfsh(void *user, void *task);

int sniff_vms_gain(void *user, void *task);

int sniff_vms_sync(void *user, void *task);

int sniff_vms_call(void *user, void *task);

int sniff_vms_exec(void *user, void *task);

int sniff_vms_idle(void *user, void *task);
