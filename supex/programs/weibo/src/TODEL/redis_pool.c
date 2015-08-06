#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "hashmap.h"
#include "redis_pool.h"
#include "utils.h"

static hashmap_t *g_hmap = NULL;

static void _connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        x_printf(W, "Error: %s\n", c->errstr);
        return;
    }
    x_printf(D, "Connected...\n");
}

static void _disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        x_printf(W, "Error: %s\n", c->errstr);
        return;
    }
    x_printf(D, "Disconnected...\n");
}

/****************************************************/

static int new_connect( struct cnt_pool *pool, uintptr_t **cite, va_list *ap )
{
	char *host = va_arg(*ap, char *);
	int port = va_arg(*ap, int);
	redisAsyncContext *c = redisAsyncConnect (host, port);
	if (!c || c->err) {
		/* Let *c leak for now... */
		x_printf (E, "Error: %s\n", c->errstr);
		free (c);
		*cite = NULL;
		return -1;
	}
	redisLibevAttach(NULL, c);
	redisAsyncSetConnectCallback(c, _connectCallback);
	redisAsyncSetDisconnectCallback(c, _disconnectCallback);
	*cite = (uintptr_t *)c;
	return 0;
}
static int del_connect( struct cnt_pool *pool, uintptr_t **cite, va_list *ap )
{
	return 0;
}

void redis_pool_init(char *host, int port)
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
	cnt_pull ( pool, &cite, host, port );
	printf("|-------%s redis pool init------->%s\n", key, (cite)?" OK!":" FAIL!");
	assert (cite);
	cnt_push ( pool, &cite );
	x_printf(D, "cite is %p\n", cite);
	return;
}

void *redis_pool_gain(struct ev_loop *loop, struct cnt_pool **pool, char *host, int port)
{
	char key[64] = {};
	sprintf(key, "%s:%d", host, port);
        size_t vlen = 0;
	hashmap_get(g_hmap, key, strlen(key), pool, &vlen);
	x_printf(D, "pool addr is %p\n", *pool);
        //printf("key:[%s] val:[%s]\n", key, v);

	if (!(*pool)){
		x_printf(E, "NO redis pool <--> %s", key);
		return NULL;
	}
	uintptr_t *cite = NULL;
	cnt_pull ( *pool, &cite, host, port );
	x_printf(D, "cite is %p\n", cite);
	if (cite){
		redisLibevEvents *e = ((struct redisAsyncContext *)cite)->ev.data;
		e->loop = loop;
		//redisLibevAttach(loop, (struct redisAsyncContext *)cite);
		//redisAsyncSetConnectCallback((struct redisAsyncContext *)cite, _connectCallback);
		//redisAsyncSetDisconnectCallback((struct redisAsyncContext *)cite, _disconnectCallback);
	}
	return cite;
}

void default_callback(redisAsyncContext *c, void *r, void *privdata)
{
    redisReply *reply = r;
    if (reply)
	    x_printf(D, "argv[%s]: %s\n", (char*)privdata, reply->str);
    if (c->err) {
	    cnt_free ( (struct cnt_pool *)privdata, (uintptr_t **)&c );
    }else{
	    redisLibevEvents *e = c->ev.data;
	    struct ev_loop *loop = e->loop;
	    if (e->reading) {
		    e->reading = 0;
		    ev_io_stop(loop, &e->rev);
	    }
	    if (e->writing) {
		    e->writing = 0;
		    ev_io_stop(loop, &e->wev);
	    }

	    cnt_push ( (struct cnt_pool *)privdata, (uintptr_t **)&c );
    }
}
