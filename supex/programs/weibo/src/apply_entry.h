#pragma once
#include <ev.h>
#include <string.h>

int entry_personal_weibo( struct ev_loop *loop, char *data, size_t size,char *p );

int entry_group_weibo( struct ev_loop *loop, char *data, size_t size );

int entry_city_weibo( struct ev_loop *loop, char *data, size_t size );
