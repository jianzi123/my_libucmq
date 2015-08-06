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



int smart_vms_init(void *user, void *task);
/*
 * 函 数:smart_vms_monitor
 * 功 能:监控系统的回调函数
 * 参 数:user 指向线程的附属结构信息, task指向任务
 * 返回值:
 * 修 改:添加新函数　程少远　2015/05/12 
 */
int smart_vms_monitor(void* user, void* task);

int smart_vms_exit(void *user, void *task);

int smart_vms_cntl(void *user, void *task);

int smart_vms_rfsh(void *user, void *task);

int smart_vms_gain(void *user, void *task);

int smart_vms_sync(void *user, void *task);

int smart_vms_call(void *user, void *task);

