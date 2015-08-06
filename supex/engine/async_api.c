#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "async_api.h"
#include "utils.h"
#include "tcp_api.h"
#include "major/major_def.h"

#define _EL_ADD_READ(ctx) do { \
	if ((ctx)->ev.add_recv) (ctx)->ev.add_recv((ctx)->ev.data); \
} while(0)
#define _EL_DEL_READ(ctx) do { \
	if ((ctx)->ev.del_recv) (ctx)->ev.del_recv((ctx)->ev.data); \
} while(0)
#define _EL_ADD_WRITE(ctx) do { \
	if ((ctx)->ev.add_send) (ctx)->ev.add_send((ctx)->ev.data); \
} while(0)
#define _EL_DEL_WRITE(ctx) do { \
	if ((ctx)->ev.del_send) (ctx)->ev.del_send((ctx)->ev.data); \
} while(0)
#define _EL_CLEANUP(ctx) do { \
	if ((ctx)->ev.cleanup) (ctx)->ev.cleanup((ctx)->ev.data); \
} while(0);

/*
static int callbackKeyCompare(void *privdata, const void *key1, const void *key2) {
	int l1, l2;
	((void) privdata);

	l1 = sdslen((const sds)key1);
	l2 = sdslen((const sds)key2);
	if (l1 != l2) return 0;
	return memcmp(key1,key2,l1) == 0;
}
*/
static struct linger g_quick_linger = {
	.l_onoff = 1,
	.l_linger = 0
};


int async_proto_redis(struct async_ctx *ac);
int async_proto_http(struct async_ctx *ac);


static struct command_node *_new_cmd_node(void)
{
	struct command_node *node = calloc(1, sizeof(struct command_node));
	assert(node);
	cache_init(&node->cache);
	return node;
}
static void _del_cmd_node(struct command_node *node)
{
	assert(node);
	cache_free(&node->cache);
	free(node);
	return;
}

static void _init_cmd_list(struct command_list *list)
{
	assert(list);
	memset(list, 0, sizeof(struct command_list));
	cache_init(&list->base.cache);
	list->head = NULL;
	list->tail = NULL;
	list->work = NULL;
	return;
}
static void _free_cmd_list(struct command_list *list)
{
	assert(list);

	struct command_node *hand = list->base.next;
	struct command_node *temp = hand;
	while(hand){
		temp = hand->next;
		_del_cmd_node(hand);
		hand = temp;
	}
	cache_free(&list->base.cache);
	list->base.fcb = NULL;
	list->base.privdata = NULL;
	list->base.next = NULL;

	list->head = NULL;
	list->tail = NULL;
	list->work = NULL;
	return;
}




struct async_ctx *async_connect(const char *host, int port)
{
	int sfd = x_connect(host, port);
	if (sfd == -1) {
		x_printf(E, "connect server(%s:%d) fail!", host, port);
		return NULL;
	}

	struct async_ctx *ac = malloc(sizeof(struct async_ctx));
	if (!ac){
		close(sfd);
		x_printf(E, "No more memory!");
		return NULL;
	}
	memset(ac, 0, sizeof(struct async_ctx));
	fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK);
	ac->sfd = sfd;
#ifdef OPEN_PUB_SUB
#else
	_init_cmd_list(&ac->replies);
#endif
	ac->parse.http_info.hp.data = ac;
	return ac;
}

int async_set_link_cb(struct async_ctx *ac, LINK_CALL_BACK *getconnect, LINK_CALL_BACK *disconnect)
{
	if (ac == NULL) {
		return ASYNC_ERR;
	}
	ac->getconnect = getconnect;
	ac->disconnect = disconnect;
	return ASYNC_OK;
}
int async_set_recy_cb(struct async_ctx *ac, RECY_CALL_BACK *fcb, void *privdata)
{
	if (ac == NULL) {
		return ASYNC_ERR;
	}
	ac->finishwork = fcb;
	ac->argv_data = privdata;
	return ASYNC_OK;
}

int async_startup(struct async_ctx *ac, enum proto_type ptype)
{
        if (ac == NULL || ac->replies.work == NULL) {
                return ASYNC_ERR;
        }

        switch(ptype) {
        case PROTO_TYPE_HTTP:
                ac->proto_handler = async_proto_http;
                break;
        case PROTO_TYPE_REDIS:
                ac->proto_handler = async_proto_redis;
                break;
        default:
                return ASYNC_ERR;
        }

	ac->getconnect(ac, ac->argv_data);
	_EL_ADD_WRITE(ac);
	return ASYNC_OK;
}

int async_distory(struct async_ctx *ac)
{
	close(ac->sfd);

	if (ac->ev.data){
		_EL_CLEANUP(ac);
	}

	_free_cmd_list(&ac->replies);

	free(ac);

	return 0;
}


static bool _run_call_back(struct async_ctx *ac, int status)
{
	struct command_node *p_work = ac->replies.work;
	if (p_work->fcb != NULL) {
		p_work->fcb(ac,
				( status == ASYNC_OK ? &p_work->cache : NULL ),
				p_work->privdata);
	}
	ac->replies.work = p_work->next;
	return ( ac->replies.work != NULL )?true:false;
}

static bool _run_call_back_once(struct async_ctx *ac, void *reply)
{
        struct command_node *p_work = ac->replies.work;
        if (p_work->fcb != NULL) {
                p_work->fcb(ac, reply, p_work->privdata);
        }
        ac->replies.work = p_work->next;
        return (ac->replies.work != NULL) ? true : false;
}

/**
 * Async receive interface.
 * Default redis protocol handler
 */
int async_proto_redis(struct async_ctx *ac)
{
        int ret;
        int sfd = ac->sfd;
        struct command_node *p_work = ac->replies.work;
        struct net_cache *p_cache = &p_work->cache;

        ret = net_recv(p_cache, sfd, &ac->control);
        if (ret > 0) {
                if (ac->control == X_MALLOC_FAILED ||
                    ac->control == X_DATA_TOO_LARGE)
                {
                        goto R_BROKEN;
                }

        } else if (ret == 0) {
                x_printf(D, "remote socket closed!socket fd: %d\n", sfd);
		setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
		goto R_BROKEN;

        } else {
                if(errno == EAGAIN ||errno == EWOULDBLOCK){
			return;
		}
		else{/* socket is going to close when reading */
			x_printf(D, "ret :%d ,close socket fd : %d\n", ret, sfd);
			setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
			goto R_BROKEN;
		}
        }

        /** analysis redis protocol from back */
        size_t reply_len;
        ret = first_reply_ok(p_cache->buf_addr+p_cache->out_size, p_cache->get_size-p_cache->out_size, &reply_len);
        if (!ret) { // continue to read
                _EL_ADD_READ(ac);
                return;
        }

        struct redis_reply* reply = proto_to_reply(p_cache->buf_addr + p_cache->out_size, reply_len);
        if (reply) {
                int have = _run_call_back_once(ac, reply);
                if (have) {
                        _EL_DEL_READ(ac);
                        _EL_ADD_WRITE(ac);
                } else {
                        _EL_DEL_READ(ac);
                        _free_cmd_list(&ac->replies);
                        ac->finishwork( ac, ac->argv_data );
                }
        }
        ac->control = X_DONE_OK;
        return;
        
R_BROKEN:
	ac->disconnect(ac, ac->argv_data);
	_run_call_back(ac, ASYNC_ERR);
	async_distory(ac);
	return;
}

/**
 * Async receive interface.
 * Default http protocol handler
 */
int async_proto_http(struct async_ctx *ac)
{
	int sfd = ac->sfd;
	struct command_node *p_work = ac->replies.work;
	struct net_cache *p_cache = &p_work->cache;

	int ret = net_recv(p_cache, sfd, &ac->control);
	if(ret > 0){
		do {
			/*no more memory*/
			if (ac->control == X_MALLOC_FAILED){
				goto R_BROKEN;
			}
			/*data too large*/
			if (ac->control == X_DATA_TOO_LARGE) {
				goto R_BROKEN;
			}

			/* parse recive data */
			if ( ! http_client_handle(ac) ){
				/*data not all,go on recive*/
				return;
			}

			/*over*/
			if (ac->parse.http_info.hs.err != HPE_OK){
				ac->control = X_PARSE_ERROR;
				goto R_BROKEN;
			}
		}while(0);
	}
	else if(ret ==0){/* socket has closed when read after */
		x_printf(D, "remote socket closed!socket fd: %d\n", sfd);
		setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
		goto R_BROKEN;
	}
	else{
		if(errno == EAGAIN ||errno == EWOULDBLOCK){
			return;
		}
		else{/* socket is going to close when reading */
			x_printf(D, "ret :%d ,close socket fd : %d\n", ret, sfd);
			setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
			goto R_BROKEN;
		}
	}

        bool have = _run_call_back_once(ac, p_cache);
	if ( have ){
		_EL_DEL_READ(ac);
		_EL_ADD_WRITE(ac);
	}else{
		_EL_DEL_READ(ac);
		_free_cmd_list(&ac->replies);
		ac->finishwork( ac, ac->argv_data );
	}
	ac->control = X_DONE_OK;
	ac->parse.http_info.hs.step = 0;

	return;
R_BROKEN:
	ac->disconnect(ac, ac->argv_data);
	_run_call_back(ac, ASYNC_ERR);
	async_distory(ac);
	return;
}

void _async_handle_recv(struct async_ctx *ac)
{
        /** http/redis */ 
        ac->proto_handler(ac);
	return;
}


void _async_handle_send(struct async_ctx *ac)
{
	int sfd = ac->sfd;
	struct command_node *p_work = ac->replies.work;
	struct net_cache *p_cache = &p_work->cache;

	int ret = net_send(p_cache, sfd, &ac->control);
	if ( ret >= 0 ){
		if (ret > 0){
			x_printf(D, "-> no all\n");
			return;
		}
		else{
			x_printf(D, "-> is all\n");
			if (ac->control < X_DONE_OK){
				goto S_BROKEN;
			}
		}
	}
	cache_free( p_cache );
	ac->control = X_DONE_OK;

	_EL_DEL_WRITE(ac);
	_EL_ADD_READ(ac);
	return;
S_BROKEN:
	ac->disconnect(ac, ac->argv_data);
	_run_call_back(ac, ASYNC_ERR);
	async_distory(ac);
	return;
}


int async_command(struct async_ctx *ac, ASYNC_CALL_BACK fcb, void *privdata, const char *data, size_t size)
{
	struct command_node *node = NULL;
	//assert(ac);
	if (ac == NULL){
		return ASYNC_ERR;
	}
	if (NULL == ac->replies.head){
		node = &ac->replies.base;
		ac->replies.head = node;
		ac->replies.tail = node;
		ac->replies.work = node;
	}else{
		node = _new_cmd_node ();
		ac->replies.tail->next = node;
		ac->replies.tail = node;
	}
	node->fcb = fcb;
	node->privdata = privdata;
	return cache_add(&node->cache, data, size);
}
