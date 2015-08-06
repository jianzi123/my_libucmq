/*
 * a simple server use libev
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "smart_evcb.h"
#include "smart_api.h"
#include "net_cache.h"
#include "http.h"
#include "redis_parser.h"

static struct timeval g_dbg_time;

extern int G_HANDER_COUNTS;
extern int G_WORKER_COUNTS;
extern int G_WORKER_COUNTS_TIMES;
extern int G_MONITOR_TIMES;
#ifdef OPEN_CORO
extern int G_TASKER_COUNTS;
extern int G_TASKER_COUNTS_TIMES;
#endif

extern MASTER_PTHREAD g_master_pthread;
extern RECVER_PTHREAD *g_recver_pthread;
extern SENDER_PTHREAD *g_sender_pthread;
extern WORKER_PTHREAD *g_worker_pthread;

extern struct smart_settings g_smart_settings;

static struct linger g_quick_linger = {
	.l_onoff = 1,
	.l_linger = 0
};
static struct linger g_delay_linger = {
	.l_onoff = 1,
	.l_linger = 1
};


extern struct api_list *smart_http_api_lookup(const char *api, int len);
extern struct cmd_list *smart_redis_cmd_lookup(int order);

extern void http_response(struct data_node *p_node, int code);
extern int http_handle(struct data_node *p_node);
/********************************************************/
static void _smart_timeout_cb(struct ev_loop *loop, ev_timer *w, int revents);
static void _smart_recv_cb(struct ev_loop *loop, ev_io *w, int revents);
static void _smart_send_cb(struct ev_loop *loop, ev_io *w, int revents);
/********************************************************/
void smart_dispatch_task(int type, int sfd)
{
	unsigned int idx = 0;
	int stp = 0;
	int mul = 1;
	HANDER_PTHREAD *p_hander = NULL;
	/*dispath to a hander thread*/
#ifdef USE_PIPE
	if (type == RECV_TASK_TYPE){
DISPATCH_RECVER_LOOP:
		idx = __sync_fetch_and_add( &g_master_pthread.robin, 1 ) % G_HANDER_COUNTS;
		p_hander = &g_recver_pthread[ idx ].base;

		if ( write(p_hander->pfds[1], (char *)&sfd, sizeof(int)) != sizeof(int) ){
			if ( 0 == ((++ stp) % (mul * SERVER_BUSY_ALARM_FACTOR)) ){
				mul ++;
				x_printf(S, "recv worker thread is too busy!\n");
			}
			goto DISPATCH_RECVER_LOOP;
		}
	}else{
		static unsigned int g_index = 0;
DISPATCH_SENDER_LOOP:
		idx = __sync_fetch_and_add(&g_index, 1) % G_HANDER_COUNTS;
		//idx = get_array_random_index( G_HANDER_COUNTS );
		p_hander = &g_sender_pthread[ idx ].base;

		if ( write(p_hander->pfds[1], (char *)&sfd, sizeof(int)) != sizeof(int) ){
			if ( 0 == ((++ stp) % (mul * SERVER_BUSY_ALARM_FACTOR)) ){
				mul ++;
				x_printf(S, "send worker thread is too busy!\n");
			}
			goto DISPATCH_SENDER_LOOP;
		}
	}
#else
	CQ_ITEM *p_item = NULL;
	struct addr_node *p_addr_node = mapping_addr_node( sfd );
	if (type == RECV_TASK_TYPE){
		idx = __sync_fetch_and_add( &g_master_pthread.robin, 1 ) % G_HANDER_COUNTS;

		p_hander = &g_recver_pthread[ idx ].base;
		p_item = &(p_addr_node->recv_item);
	}else{
		static unsigned int g_index = 0;
		idx = __sync_fetch_and_add(&g_index, 1) % G_HANDER_COUNTS;

		p_hander = &g_sender_pthread[ idx ].base;
		p_item = &(p_addr_node->send_item);
	}
ACCEPT_LOOP:
	if ( !ev_async_pending(&(p_hander->async_watcher)) ) {
		// the event has not yet been processed (or even noted) by the event
		// loop? (i.e. Is it serviced? If yes then proceed to)
		// Sends/signals/activates the given ev_async watcher, that is, feeds
		// an EV_ASYNC event on the watcher into the event loop.
		cq_push( &(p_hander->qlist) , p_item);
		ev_async_send(p_hander->loop, &(p_hander->async_watcher));
	}else{
		x_printf(D, "IN ACCEPT_LOOP!\n");
		goto ACCEPT_LOOP;
	}
#endif

}

void smart_all_task_hit( struct smart_task_node *task, bool synch, int mark )
{
	int i = 0;
	int id = 0;
	char deal = TASK_IS_BEGIN;

	task->type	= BIT8_TASK_TYPE_WHOLE;
	task->deal	= synch ? &deal : NULL;
	do {
#ifdef OPEN_CORO
		id = smart_task_rgst(task->origin, task->type, G_WORKER_COUNTS, G_TASKER_COUNTS, mark);
#else
		id = smart_task_rgst(task->origin, task->type, G_WORKER_COUNTS, G_WORKER_COUNTS, mark);
#endif
		if ( id < 0 ) {
			x_printf(I, "WARNING: Too much manage task not done origin from %c!\n", task->origin);
			sched_yield();//usleep(0);
		}else {
			task->id = id;

			for (i = 0; i < G_WORKER_COUNTS; ++i) {
				x_printf(D, "worker id %d\n", i);
				while ( false == g_smart_settings.conf->task_report( &g_worker_pthread[ i ], task ) ){
					sched_yield();//usleep(0);
				};
			}
		}
	} while( id < 0 );
	if (synch && task->func){
		while( deal != TASK_IS_FINISH ){
			sched_yield();//usleep(0);
		}
	}
}

void smart_one_task_hit( struct smart_task_node *task, bool synch, int mark, unsigned int step )
{
	int id = 0;
	char deal = TASK_IS_BEGIN;

	task->type	= BIT8_TASK_TYPE_ALONE;
	task->deal	= synch ? &deal : NULL;
	do {
#ifdef OPEN_CORO
		id = smart_task_rgst(task->origin, task->type, G_WORKER_COUNTS, G_TASKER_COUNTS, mark);
#else
		id = smart_task_rgst(task->origin, task->type, G_WORKER_COUNTS, G_WORKER_COUNTS, mark);
#endif
		if ( id < 0 ) {
			x_printf(I, "WARNING: Too much manage task not done origin from %c!\n", task->origin);
			sched_yield();//usleep(0);
		}else {
			task->id = id;

			x_printf(D, "worker id %d\n", step);
			while ( false == g_smart_settings.conf->task_report( &g_worker_pthread[ step ], task ) ){
				sched_yield();//usleep(0);
			};
		}
	} while( id < 0 );
	if (synch && task->func){
		while( deal != TASK_IS_FINISH ){
			sched_yield();//usleep(0);
		}
	}
}

void smart_accept_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	int newfd;

	/*accept*/
	struct sockaddr_in sin;
	socklen_t addrlen = sizeof(struct sockaddr);
	while ((newfd = accept(w->fd, (struct sockaddr *)&sin, &addrlen)) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			/* these are transient, so don't log anything. */
			continue; 
		}
		else {
			x_printf(D, "accept error.[%s]\n", strerror(errno));
			return;
		}
	}
	if (newfd >= MAX_LIMIT_FD){
		x_printf(D, "error : this fd (%d) too large!\n", newfd);
		send(newfd, FETCH_MAX_CNT_MSG, strlen(FETCH_MAX_CNT_MSG), 0);
		//setsockopt(newfd, SOL_SOCKET, SO_LINGER, (const char *)&g_delay_linger, sizeof(g_delay_linger));
		close(newfd);
		return;
	}//FIXME how to message admin
	x_out_time(&g_dbg_time);
	/*set status*/
	fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFL) | O_NONBLOCK);
	/*set the new connect*/
	struct addr_node *p_addr_node = mapping_addr_node( newfd );
	p_addr_node->port = ntohs(sin.sin_port);
	memset(p_addr_node->szAddr, 0, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &sin.sin_addr, p_addr_node->szAddr, INET_ADDRSTRLEN);

	smart_dispatch_task(RECV_TASK_TYPE, newfd);
}

static int fetch_link_work_task(HANDER_PTHREAD *p_hander, struct ev_loop *loop)
{
	CQ_ITEM *item = cq_pop( &(p_hander->qlist) );

	if (item != NULL) {
		int sfd = item->data;
		struct addr_node *p_addr_node = mapping_addr_node( sfd );
		if( __sync_bool_compare_and_swap(&p_addr_node->work_status, NO_WORKING, IS_WORKING) ){
			struct data_node *p_node = get_pool_addr( sfd );
			p_node->io_watcher.data = p_hander;
			ev_io_init( &(p_node->io_watcher), _smart_recv_cb, p_node->sfd, EV_READ);
			ev_io_start( loop, &(p_node->io_watcher) );
			// x_printf(D, "thread[%lu] accept: fd :%d  addr:%s port:%d\n", p_hander->thread_id, p_node->sfd, p_node->szAddr, p_node->port);
#ifdef OPEN_TIME_OUT
			// x_printf(D, "start timer!\n");
			ev_timer *p_timer = &(p_node->timer_watcher);;
			ev_timer_init(p_timer, _smart_timeout_cb, 5.5, 0. );
			ev_timer_start(loop, p_timer);
#endif
			item_init(item);
			return 1;
		}
		else {
			cq_push( &(p_hander->qlist), item );
			return 0;
		}
	}
	return 0;
}

static int fetch_link_over_task(HANDER_PTHREAD *p_hander, struct ev_loop *loop)
{
	CQ_ITEM *item = cq_pop( &(p_hander->qlist) );

	if (item != NULL) {
		int sfd = item->data;
		struct data_node *p_node = get_pool_addr( sfd );
		
		p_node->io_watcher.data = p_hander;
		ev_io_init( &(p_node->io_watcher), _smart_send_cb, p_node->sfd, EV_WRITE );
		ev_io_start( loop, &(p_node->io_watcher) );
		// x_printf(D, "thread[%lu] accept: fd :%d  addr:%s port:%d\n", p_hander->thread_id, p_node->sfd, p_node->szAddr, p_node->port);
		item_init(item);
		return 1;
	}
	return 0;
}

void smart_prepare_cb(struct ev_loop *loop, ev_prepare *w, int revents)
{
	x_printf(D, "in prepare callback \n");
}

#ifdef USE_PIPE
static int fetch_pipe_work_task(HANDER_PTHREAD *p_hander, struct ev_loop *loop)
{
	int sfd = 0;
	int n = read(p_hander->pfds[0], (char *)&sfd, sizeof(int));
	assert( (n == -1) || (n == sizeof(int)) );
	if (n == sizeof(int)){
		struct addr_node *p_addr_node = mapping_addr_node( sfd );
		if( __sync_bool_compare_and_swap(&p_addr_node->work_status, NO_WORKING, IS_WORKING) ){
			struct data_node *p_node = get_pool_addr( sfd );
			p_node->io_watcher.data = p_hander;
			ev_io_init( &(p_node->io_watcher), _smart_recv_cb, p_node->sfd, EV_READ);
			ev_io_start( loop, &(p_node->io_watcher) );
			// x_printf(D, "thread[%lu] accept: fd :%d  addr:%s port:%d\n", p_hander->thread_id, p_node->sfd, p_node->szAddr, p_node->port);
#ifdef OPEN_TIME_OUT
			// x_printf(D, "start timer!\n");
			ev_timer *p_timer = &(p_node->timer_watcher);;
			ev_timer_init(p_timer, _smart_timeout_cb, 5.5, 0. );
			ev_timer_start(loop, p_timer);
#endif
			return 1;
		}
		else {
			cq_push( &(p_hander->qlist), &(p_addr_node->recv_item) );
			return 0;
		}
	}
	return 0;
}
static int fetch_pipe_over_task(HANDER_PTHREAD *p_hander, struct ev_loop *loop)
{
	int sfd = 0;
	int n = read(p_hander->pfds[0], (char *)&sfd, sizeof(int));
	assert( (n == -1) || (n == sizeof(int)) );
	if (n == sizeof(int)){
		struct data_node *p_node = get_pool_addr( sfd );

		p_node->io_watcher.data = p_hander;
		ev_io_init( &(p_node->io_watcher), _smart_send_cb, p_node->sfd, EV_WRITE );
		ev_io_start( loop, &(p_node->io_watcher) );
		// x_printf(D, "thread[%lu] accept: fd :%d  addr:%s port:%d\n", p_hander->thread_id, p_node->sfd, p_node->szAddr, p_node->port);
		return 1;
	}
	return 0;
}

void smart_fetch_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	HANDER_PTHREAD *p_hander = (HANDER_PTHREAD *)(w->data);
	if (p_hander->type == RECV_TASK_TYPE)
		fetch_pipe_work_task(p_hander, loop);
	else
		fetch_pipe_over_task(p_hander, loop);
}

void smart_check_cb(struct ev_loop *loop, ev_check *w, int revents)
{
	int idx = 0;
	HANDER_PTHREAD *p_hander = (HANDER_PTHREAD *)(w->data);
	if (p_hander->type == RECV_TASK_TYPE){
		while( !!fetch_link_work_task(p_hander, loop) ){ 
			idx++;
		}
		if (idx == 0){
			fetch_pipe_work_task(p_hander, loop);
		}
	}else{
		while( !!fetch_link_over_task(p_hander, loop) ){ 
			idx++;
		}
		if (idx == 0){
			fetch_pipe_over_task(p_hander, loop);
		}
	}
}
#else
void smart_async_cb(struct ev_loop *loop, ev_async *w, int revents)
{
	HANDER_PTHREAD *p_hander = (HANDER_PTHREAD *)(w->data);
	if (p_hander->type == RECV_TASK_TYPE){
		fetch_link_work_task(p_hander, loop);
	}else{
		fetch_link_over_task(p_hander, loop);
	}
}

void smart_check_cb(struct ev_loop *loop, ev_check *w, int revents)
{
	HANDER_PTHREAD *p_hander = (HANDER_PTHREAD *)(w->data);
	if (p_hander->type == RECV_TASK_TYPE){
		while( !!fetch_link_work_task(p_hander, loop) ){ }
	}else{
		while( !!fetch_link_over_task(p_hander, loop) ){ }
	}
}
#endif


static void _smart_recv_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	struct data_node *p_node = get_pool_addr( w->fd );
	int ret = net_recv(&p_node->recv, w->fd, &p_node->control);
	int sfd = w->fd;

	x_out_time(&g_dbg_time);
	if(ret > 0){
		do {
#ifdef USE_HTTP_PROTOCOL
			/*no more memory*/
			if (p_node->control == X_MALLOC_FAILED){
				http_response(p_node, 500);
				break;
			}
			/*data too large*/
			if (p_node->control == X_DATA_TOO_LARGE) {
				http_response(p_node, 413);
				break;
			}

			/* parse recive data */
			if ( ! http_handle(p_node) ){
				/*data not all,go on recive*/
				return;
			}
			/*over*/
			if (p_node->http_info.hs.err != HPE_OK){
				http_response(p_node, 503);
				p_node->control = X_PARSE_ERROR;
				break;
			}
			/*parse path*/
			struct api_list *p_api = smart_http_api_lookup( p_node->recv.buf_addr + p_node->http_info.hs.path_offset,
					p_node->http_info.hs.path_offset );
			if ( p_api == NULL){
				http_response(p_node, 400);
				p_node->control = X_REQUEST_ERROR;
				break;
			}

			/*make api task*/
			struct smart_task_node task = {
				.id	= sfd,
				.sfd	= sfd,
				.type	= p_api->type,
				.origin	= BIT8_TASK_ORIGIN_HTTP,
				.func	= p_api->func,
				.index	= 0,
				.data	= NULL
			};
			/*reset parse*/
			p_node->http_info.hs.step = 0;
#endif
#ifdef USE_REDIS_PROTOCOL
			/*no more memory*/
			if (p_node->control == X_MALLOC_FAILED){
				cache_add( &p_node->send, OPT_NO_MEMORY, strlen(OPT_NO_MEMORY) );
				break;
			}
			/*data too large*/
			if (p_node->control == X_DATA_TOO_LARGE) {
				cache_add( &p_node->send, OPT_DATA_TOO_LARGE, strlen(OPT_DATA_TOO_LARGE) );
				break;
			}

			/* parse recive data */
			p_node->control = rds_cmd_parse(p_node);
			switch (p_node->control){
				case X_DONE_OK:
				case X_DATA_NO_ALL:
					/*data not all,go on recive*/
					return;
				case X_DATA_IS_ALL:
					break;
				case X_PARSE_ERROR:
					cache_add( &p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR) );
					break;
				case X_KV_TOO_MUCH:
					cache_add( &p_node->send, OPT_KV_TOO_MUCH, strlen(OPT_KV_TOO_MUCH) );
					break;
				case X_NAME_TOO_LONG:
					cache_add( &p_node->send, OPT_NAME_TOO_LONG, strlen(OPT_NAME_TOO_LONG) );
					break;
				case X_REQUEST_ERROR:
					cache_add( &p_node->send, OPT_NO_THIS_CMD, strlen(OPT_NO_THIS_CMD) );
					break;
				case X_REQUEST_QUIT:
					setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
					goto R_BROKEN;
				default:
					//cache_add( &p_node->send, OPT_UNKNOW_ERROR, strlen(OPT_UNKNOW_ERROR) );
					setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
					goto R_BROKEN;
			}
			/*over*/
			if (p_node->control != X_DATA_IS_ALL){
				/*jump to send error*/
				break;
			}

			struct cmd_list *p_cmd = smart_redis_cmd_lookup( p_node->redis_info.rs.order );
			if ( p_cmd == NULL){
				p_node->control = X_INTERIOR_ERROR;
				cache_add( &p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR) );
				break;
			}
			/*make cmd task*/
			struct smart_task_node task = {
				.id	= sfd,
				.sfd	= sfd,
				.type	= p_cmd->type,
				.origin	= BIT8_TASK_ORIGIN_REDIS,
				.func	= p_cmd->func,
				.index	= 0,
				.data	= NULL
			};
			/*reset parse*/
			rds_reset_parser(p_node);
#endif
			/*
			 * can only restart recv until this task done,
			 * should add stop code before task start,
			 * so can't add after task done,because we use asynchronous mechanism.
			 */
			ev_io_stop(loop,  w);
			
			switch ( task.type ){
				case BIT8_TASK_TYPE_ALONE:
					{
						RECVER_PTHREAD *p_recver = w->data;
						unsigned int offset = __sync_fetch_and_add( &p_recver->robin, 1 ) % G_WORKER_COUNTS_TIMES;
						unsigned int start = G_WORKER_COUNTS_TIMES * p_recver->index;
						unsigned int step = start + offset;

						x_printf(D, "hander id %d worker id %d\n",p_recver->index, step);
						smart_one_task_hit( &task, false, sfd, step );
					}
					break;
				case BIT8_TASK_TYPE_WHOLE:
					smart_all_task_hit( &task, false, sfd );
					break;
				default:
					x_printf(D, "Error task type %c\n", task.type);
					return;

			}
			return;
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
#ifdef USE_HTTP_PROTOCOL
	p_node->http_info.hs.step = 0;
#endif
#ifdef USE_REDIS_PROTOCOL
	rds_reset_parser(p_node);
#endif
	cache_free( &p_node->recv );
	ev_io_stop(loop,  w);
	ev_io_init(w, _smart_send_cb, sfd, EV_WRITE);
	ev_io_start(loop, w);
	/* it will quickly run into  _smart_send_cb() when socket is not broken */
	return;
R_BROKEN:
#ifdef OPEN_TIME_OUT
	ev_timer_stop( loop, &(p_node->timer_watcher) );
#endif
#ifdef USE_HTTP_PROTOCOL
	p_node->http_info.hs.step = 0;
#endif
#ifdef USE_REDIS_PROTOCOL
	rds_reset_parser(p_node);
#endif
	cache_free( &p_node->recv );
	p_node->control = X_DONE_OK;
	close(sfd);
	ev_io_stop(loop, w);

	del_pool_addr( sfd );
	struct addr_node *p_addr_node = mapping_addr_node( sfd );
	assert( __sync_bool_compare_and_swap(&p_addr_node->work_status, IS_WORKING, NO_WORKING) );
	return;
}



static void _smart_send_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	x_out_time(&g_dbg_time);
	int sfd = w->fd;

	struct data_node *p_node = get_pool_addr( sfd );
	int ret = net_send(&p_node->send, sfd, &p_node->control);
	if ( ret >= 0 ){
		if (ret > 0){
			x_printf(D, "-> no all\n");
			return;
		}
		else{
			x_printf(D, "-> is all\n");
#ifdef USE_HTTP_PROTOCOL
			if ( (p_node->control != X_DONE_OK) || !(p_node->http_info.hs.should_keep_alive) ){
				goto S_BROKEN;
			}
#endif
#ifdef USE_REDIS_PROTOCOL
			if (p_node->control < X_DONE_OK){
				goto S_BROKEN;
			}
#endif
		}
	}
#ifdef OPEN_TIME_OUT
	goto S_BROKEN;
#else
	cache_free( &p_node->send );
	p_node->control = X_DONE_OK;
	ev_io_stop(loop,  w);
	ev_io_init(w, _smart_recv_cb, sfd, EV_READ);
	ev_io_start(loop, w);
	return;
#endif
S_BROKEN:
#ifdef OPEN_TIME_OUT
	ev_timer_stop( loop, &(p_node->timer_watcher) );
#endif
	cache_free( &p_node->send );
	p_node->control = X_DONE_OK;
	//setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&g_delay_linger, sizeof(g_delay_linger));//FIXME:no effect when O_NONBLOCK
	close(sfd);
	ev_io_stop(loop, w);

	del_pool_addr( sfd );
	struct addr_node *p_addr_node = mapping_addr_node( sfd );
	assert( __sync_bool_compare_and_swap(&p_addr_node->work_status, IS_WORKING, NO_WORKING) );
	return;
}

static void _smart_timeout_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	struct data_node *p_node = w->data;
	ev_io *p_watcher = &(p_node->io_watcher);
	int sfd = p_watcher->fd;

	// x_printf(D, "timeout\n");	
	cache_free( &p_node->recv );
	cache_free( &p_node->send );
	setsockopt(sfd, SOL_SOCKET, SO_LINGER, (const char *)&g_quick_linger, sizeof(g_quick_linger));
	close( sfd );
	ev_io_stop( loop , p_watcher );

	del_pool_addr( sfd );
	struct addr_node *p_addr_node = mapping_addr_node( sfd );
	assert( __sync_bool_compare_and_swap(&p_addr_node->work_status, IS_WORKING, NO_WORKING) );

	/* break this one connect */
	//ev_break(EV_A_ EVBREAK_ONE);

	/* clear timer from loop */
	//ev_unloop (EV_A_ EVUNLOOP_ONE); 
}

/*
 * 名  称:smart_monitor_cb
 * 功  能:周期循环事件的回调函数
 * 参  数:loop 事件循环，　w　周期循环事件，　
 * 返回值:
 * 修  改:添加新函数,程少远 2015/05/12
 */
void smart_monitor_cb(struct ev_loop* loop, ev_periodic* w, int revents)
{
        struct smart_task_node task = {
		.id	= 0,
		.sfd	= 0,
		.type	= BIT8_TASK_TYPE_WHOLE,
		.origin	= BIT8_TASK_ORIGIN_TIME,
		.func	= g_smart_settings.conf->vmsys_monitor,
		.index	= 0,
		.data	= NULL
	};
        //异步方式向所有的工作线程中添加任务
	smart_all_task_hit( &task, false, 0 );
}

void smart_update_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	int space = get_overplus_time();
	if (0 == space){
		x_printf(S, "update ...\n");
		/*update c logfd*/
		open_new_log();
		/*update lua logfd*/
		struct smart_task_node task = {
			.id	= 0,
			.sfd	= 0,
			.type	= BIT8_TASK_TYPE_WHOLE,
			.origin	= BIT8_TASK_ORIGIN_TIME,
			.func	= g_smart_settings.conf->vmsys_rfsh,
			.index	= 0,
			.data	= NULL
		};
		smart_all_task_hit( &task, false, 0 );

		/*reget timer*/
		space = get_overplus_time();
	}
	/*reset timer*/
	ev_timer_stop( loop, w );
	ev_timer_init( w, smart_update_cb, space, 0. );
	ev_timer_start( loop, w );
}

void smart_signal_cb (struct ev_loop *loop, ev_signal *w, int revents)
{
	x_printf(D, "get a signal\n");
	if(w->signum == SIGQUIT){
		//TODO free pool buff
		ev_signal_stop( loop, w );
		ev_break (loop, EVBREAK_ALL);
	}
}
