#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "cnt_pool.h"

int cnt_init ( struct cnt_pool *pool, unsigned int max,
		CNT_SOCK_OPT cso_open, CNT_SOCK_OPT cso_exit )
{
	memset(pool, 0, sizeof(struct cnt_pool));
	pool->max = max;
	pool->use = 0;
	pool->cso_open = cso_open;
	pool->cso_exit = cso_exit;
	free_queue_init(&pool->qlist, sizeof(uintptr_t *), max);
	assert( cso_open && cso_exit );
	return 0;
}

int cnt_pull ( struct cnt_pool *pool, uintptr_t **cite, ... )
{
	bool ok = false;
	unsigned int idx = 0;

PULL_START:
	if( free_queue_pull(&pool->qlist, cite) ){
		return 0;
	}
	do {
		idx = pool->use;
#if 0
		if (idx == pool->max){
			usleep(1000);
			break;
		}
#endif
		ok = __sync_bool_compare_and_swap(&pool->use, idx, idx + 1);
	}while( !ok );
	if (!ok){
		goto PULL_START;
	}
	va_list ap;
	va_start (ap, cite);
	int ret = pool->cso_open (pool, cite, &ap);
	va_end (ap);
	return ret;
}

int cnt_push ( struct cnt_pool *pool, uintptr_t **cite )
{
	assert( free_queue_push(&pool->qlist, cite) );
	return 0;
}

int cnt_free ( struct cnt_pool *pool, uintptr_t **cite, ... )
{
	assert( __sync_fetch_and_sub(&pool->use, 1) );
	va_list ap;
	va_start (ap, cite);
	pool->cso_exit (pool, cite, &ap);
	va_end (ap);
	return 0;
}
