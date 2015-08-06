#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "net_cache.h"
#include "major_def.h"

unsigned int g_max_req_size = MAX_REQ_SIZE;

/*
 *函 数:cache_peak
 *功 能:判断max是否大于g_max_req_size,如果大于则把g_max_req_size设置为max　
 *参 数:
 *返回值:
 */
void cache_peak(unsigned int max)
{
	unsigned int old = 0;
	unsigned int tmp = 0;
	unsigned int new = 0;

	do {
		old = g_max_req_size;
		tmp = MAX(max, old);
		new = (tmp > MAX_REQ_SIZE) ? MAX_REQ_SIZE : tmp;
	}
	while ( !__sync_bool_compare_and_swap(&g_max_req_size, old, new) );
	return;
}

void cache_init(struct net_cache *info)
{
	memset( info, 0, sizeof(struct net_cache) );

	info->buf_addr = info->base;
	info->max_size = MAX_DEF_LEN;
	return;
}

int cache_add(struct net_cache *info, const char *data, int size)
{
	char *old = info->buf_addr;
	assert( info->buf_addr && size > 0 );
	int all_size = info->get_size + size;
	if ( info->max_size < all_size ){
		info->max_size = GET_NEED_COUNT( all_size, MAX_DEF_LEN ) * MAX_DEF_LEN;
		if ( ! (info->buf_addr = malloc(info->max_size)) ){
			return X_MALLOC_FAILED;
		}
		memset(info->buf_addr, 0,  info->max_size);
		memcpy(info->buf_addr, old, info->get_size);
		if (old == info->base){
			memset(info->base, 0, MAX_DEF_LEN);
		}else{
			free(old);
		}
	}
	memcpy(info->buf_addr + info->get_size, data, size);
	info->get_size = all_size;
	return X_DONE_OK;
}

ssize_t net_recv(struct net_cache *cache, int sockfd, int *status)
{
	int ret = 0;
	char temp[MAX_DEF_LEN] = {0};
	//FIXME : while
	
	ret = recv(sockfd, temp, MAX_DEF_LEN, 0);
	x_printf(D, "%d     recv size : %d\n", sockfd, ret);

	if (ret > 0){
		if ( g_max_req_size < (cache->get_size + ret) ){
			*status = X_DATA_TOO_LARGE;
		}else{
			*status = cache_add(cache, temp, ret);
		}
	}

	return ret;
}

ssize_t net_send(struct net_cache *cache, int sockfd, int *status)
{
	if (cache->get_size != cache->out_size){
		int ret = send(sockfd, cache->buf_addr + cache->out_size,
				cache->get_size - cache->out_size,
				0);
		x_printf(D, "-------> send = %d\n",  ret);

		if (ret < 0){
			*status = X_IO_ERROR;
			x_printf(I, "discard all\n");
			return 0;
		}else{
			cache->out_size += ret;
			if (cache->get_size != cache->out_size){
				x_printf(I, "no all\n");
				return (cache->get_size - cache->out_size);
			}
			else{
				x_printf(I, "is all\n");
				return 0;
			}
		}
	}
	return -1;
}

int cache_set(struct net_cache *info, const char *data, int max, int size)
{
	if (size > 0) {
		if (info->buf_addr != info->base){
			free(info->buf_addr);
		}
		info->max_size = max;
		info->buf_addr = (char *)data;
		info->get_size = size;
	}
	return X_DONE_OK;
}

void cache_free(struct net_cache *info)
{
	info->max_size = MAX_DEF_LEN;
	info->get_size = 0;
	info->out_size = 0;
	if (info->buf_addr != info->base){
		free( info->buf_addr );
		info->buf_addr = info->base;
	}
	memset(info->base, 0, MAX_DEF_LEN);
	return;
}
