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



int crzpt_vms_init( void *user, void *task, int step );

int crzpt_vms_exit( void *user, void *task, int step );

int crzpt_vms_load( void *user, void *task, int step );

int crzpt_vms_rfsh( void *user, void *task, int step );

int crzpt_vms_call( void *user, void *task, int step );

int crzpt_vms_push( void *user, void *task, int step );

int crzpt_vms_pull( void *user, void *task, int step );
