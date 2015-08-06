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



int smart_vms_init( int I, void *T, void *W );

int smart_vms_exit( int I, void *T, void *W );

int smart_vms_cntl( int I, void *T, void *W );

int smart_vms_rfsh( int I, void *T, void *W );

int smart_vms_gain( int I, void *T, void *W );

int smart_vms_sync( int I, void *T, void *W );

int smart_vms_call( int I, void *T, void *W );

int smart_vms_exec( int I, void *T, void *W );
