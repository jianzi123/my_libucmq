#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pool_api.h"
#include "async_api.h"
#include "utils.h"
#include "hashmap.h"
#include "async_libev.h"

static hashmap_t *g_hmap = NULL;

static void _get_connect_cb(const struct async_ctx *ac, void *data)
{
	x_printf(I, "Connected...\n");
}

static void _dis_connect_cb(const struct async_ctx *ac, void *data)
{
	x_printf(W, "Disconnected...\n");
	cnt_free ( (struct cnt_pool *)data, (uintptr_t **)&ac );
}

static void _recy_pool_cb(const struct async_ctx *ac, void *data)
{
#if 1
	cnt_push ( (struct cnt_pool *)data, (uintptr_t **)&ac );
#else
	cnt_free ( (struct cnt_pool *)data, (uintptr_t **)&ac );
	async_distory(ac);
#endif
}

/****************************************************/

static int new_connect( struct cnt_pool *pool, uintptr_t **cite, va_list *ap )
{
	char *host = va_arg(*ap, char *);
	int port = va_arg(*ap, int);

	struct async_ctx *ac = async_connect (host, port);
	if (ac == NULL) {
		x_printf (E, "Error: async_connect\n");
		*cite = NULL;
		return -1;
	}
	int ok = async_init(ac);
	if (ok == ASYNC_ERR) {
		x_printf (E, "Error: async_init\n");
		async_distory(ac);
		*cite = NULL;
		return -1;
	}

	ok = async_set_link_cb(ac, _get_connect_cb, _dis_connect_cb);
	ok = async_set_recy_cb(ac, _recy_pool_cb, pool);
	*cite = (uintptr_t *)ac;
	return 0;
}
static int del_connect( struct cnt_pool *pool, uintptr_t **cite, va_list *ap )
{
	return 0;
}




void pool_api_init(char *host, int port)
{
	char key[64] = {};
	if (!g_hmap){
		g_hmap = hashmap_open();
		assert(g_hmap != NULL);
	}

	struct cnt_pool *pool = calloc(1, sizeof(struct cnt_pool));
	cnt_init ( pool, 20000, new_connect, del_connect );
	sprintf(key, "%s:%d", host, port);
	x_printf(D, "pool addr is %p\n", pool);
	hashmap_set(g_hmap, key, strlen(key), &pool, sizeof(uintptr_t *));

	uintptr_t *cite = NULL;
	int ok = cnt_pull ( pool, &cite, host, port );
	printf("|-------%s pool init------->%s\n", key, (cite)?" OK!":" FAIL!");
	assert (cite);
	cnt_push ( pool, &cite );
	x_printf(D, "cite is %p\n", cite);
	return;
}

void *pool_api_gain(struct ev_loop *loop, struct cnt_pool **pool, char *host, int port)
{
	char key[64] = {};
	sprintf(key, "%s:%d", host, port);
	size_t vlen = 0;
	hashmap_get(g_hmap, key, strlen(key), pool, &vlen);
	x_printf(D, "pool addr is %p\n", *pool);
	//printf("key:[%s] val:[%s]\n", key, v);

	if (!(*pool)){
		x_printf(E, "NO pool <--> %s", key);
		return NULL;
	}
	uintptr_t *cite = NULL;
	int ok = cnt_pull ( *pool, &cite, host, port );
	x_printf(D, "cite is %p\n", cite);
	if (cite){
		async_bind(loop, (struct async_ctx *)cite);
	}
	return cite;
}
