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



int swift_vms_init( void *W );

int swift_vms_exit( void *W );

int swift_vms_cntl( void *W );

int swift_vms_rfsh( void *W );

int swift_vms_gain( void *W );

int swift_vms_sync( void *W );

int swift_vms_call( void *W );

int swift_vms_exec( void *W );
