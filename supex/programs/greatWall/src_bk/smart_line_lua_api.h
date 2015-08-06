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



int smart_vms_init( void *W );

int smart_vms_exit( void *W );

int smart_vms_cntl( void *W );

int smart_vms_rfsh( void *W );

int smart_vms_gain( void *W );

int smart_vms_sync( void *W );

int smart_vms_call( void *W );

int smart_vms_exec( void *W );
