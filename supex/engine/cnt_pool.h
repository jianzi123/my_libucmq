#pragma once

#include <stdint.h>

#include "utils.h"

struct cnt_pool;
typedef int (*CNT_SOCK_OPT)( struct cnt_pool *pool, uintptr_t **cite, va_list *ap );

struct cnt_pool {
	unsigned int max;
	unsigned int use;

	CNT_SOCK_OPT cso_open;
	CNT_SOCK_OPT cso_exit;
	struct free_queue_list qlist;
};

int cnt_init ( struct cnt_pool *pool, unsigned int max,
		CNT_SOCK_OPT cso_open, CNT_SOCK_OPT cso_exit );

int cnt_pull ( struct cnt_pool *pool, uintptr_t **cite, ... );

int cnt_push ( struct cnt_pool *pool, uintptr_t **cite );

int cnt_free ( struct cnt_pool *pool, uintptr_t **cite, ... );
