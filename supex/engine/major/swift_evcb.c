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

#include "swift_evcb.h"
#include "swift_api.h"
#include "net_cache.h"
#include "http.h"
#include "redis_parser.h"

static struct timeval g_dbg_time;

extern int G_PORTER_COUNTS;

extern LEADER_PTHREAD g_leader_pthread;
extern PORTER_PTHREAD *g_porter_pthread;

extern struct swift_settings g_swift_settings;

static struct linger g_quick_linger = {
	.l_onoff = 1,
	.l_linger = 0
};
static struct linger g_delay_linger = {
	.l_onoff = 1,
	.l_linger = 1
};

#ifdef USE_HTTP_PROTOCOL
extern void http_response(struct data_node *p_node, int code);
extern int http_handle(struct data_node *p_node);
extern struct api_list *swift_http_api_lookup(const char *api, int len);
#endif
#ifdef USE_REDIS_PROTOCOL
extern int rds_cmd_parse(struct data_node *p_node);
extern int rds_reset_parser(struct data_node *p_node);
extern struct cmd_list *swift_redis_cmd_lookup(int order);
#endif
/********************************************************/
static void _swift_timeout_cb(struct ev_loop *loop, ev_timer *w, int revents);
static void _swift_recv_cb(struct ev_loop *loop, ev_io *w, int revents);
static void _swift_send_cb(struct ev_loop *loop, ev_io *w, int revents);
/********************************************************/
/*
 *函 数:swift_dispatch_task
 *功 能:主线程收到一个newfd之后会调用该函数，把newfd分配给某个porter线程
 */
static void swift_dispatch_task(int sfd)
{
	unsigned int idx = 0;
	int stp = 0;
	int mul = 1;
	PORTER_PTHREAD *p_porter = NULL;
	/*dispath to a hander thread*/
	//如果使用管道的话，会按顺序依次向各个porter线程的管道中写入一个newfd
#ifdef USE_PIPE
DISPATCH_LOOP:
	idx = __sync_fetch_and_add( &g_leader_pthread.robin, 1 ) % G_PORTER_COUNTS;
	p_porter = &g_porter_pthread[ idx ];

	if ( write(p_porter->pfds[1], (char *)&sfd, sizeof(int)) != sizeof(int) ){
		if ( 0 == ((++ stp) % (mul * SERVER_BUSY_ALARM_FACTOR)) ){
			mul ++;
			x_printf(S, "recv worker thread is too busy!\n");
		}
		goto DISPATCH_LOOP;
	}
#else
	//不用管道的话
	idx = __sync_fetch_and_add( &g_leader_pthread.robin, 1 ) % G_PORTER_COUNTS;
	p_porter = &g_porter_pthread[ idx ];

	struct addr_node *p_addr_node = mapping_addr_node( sfd );
	//p_item指向sfd对应的槽位的recv_item内存
	CQ_ITEM *p_item = &(p_addr_node->recv_item);
DISPATCH_LOOP:
#if 1
	//添加到p_porter的qlist队列中，qlist是通过链表和互斥锁实现的安全队列
	cq_push( &(p_porter->qlist) , p_item);
	//触发p_porter线程的async_watcher事件
	ev_async_send(p_porter->loop, &(p_porter->async_watcher));
#else
	if ( !ev_async_pending(&(p_porter->async_watcher)) ) {
		// the event has not yet been processed (or even noted) by the event
		// loop? (i.e. Is it serviced? If yes then proceed to)
		// Sends/signals/activates the given ev_async watcher, that is, feeds
		// an EV_ASYNC event on the watcher into the event loop.
		cq_push( &(p_porter->qlist) , p_item);
		ev_async_send(p_porter->loop, &(p_porter->async_watcher));
	}else{
		x_printf(D, "IN DISPATCH_LOOP!\n");
		goto DISPATCH_LOOP;
	}
#endif
#endif

}

/*
 *函 数:task_handle
 *功 能:处理任务，处理完之后给以反馈
 */
static void task_handle(PORTER_PTHREAD *p_porter)
{
	if ( p_porter->task.func == NULL ){
		x_printf(S, "TASK is NULL function!\n");
		return;
	}
	/*call api*/
	//调用任务的回调处理函数
	int err = p_porter->task.func( p_porter );
	
	//任务执行完
	if ( 0 == swift_task_over( p_porter->task.id ) ){
		struct swift_task_node *p_task = &p_porter->task;
		if (p_task->deal){
			*(p_task->deal) = TASK_IS_FINISH;
		}
		if (p_task->data != NULL){
			switch (p_task->origin){
				case BIT8_TASK_ORIGIN_MSMQ:
					free(p_task->data);
					break;
				default:
					break;
			}
			p_task->data = NULL;
		}
#ifdef USE_HTTP_PROTOCOL
		if ( p_porter->task.origin == BIT8_TASK_ORIGIN_HTTP ){
			struct data_node *p_node = get_pool_addr( p_porter->task.sfd );
			/*check error*/
			if (err) {
				http_response(p_node, 500);
				p_node->control = X_EXECUTE_ERROR;
			}else{
				http_response(p_node, 200);
				p_node->control = X_DONE_OK;
			}
			cache_free( &p_node->recv );
			ev_io_init(&(p_node->io_watcher), _swift_send_cb, p_node->sfd, EV_WRITE);
			ev_io_start(p_porter->loop, &(p_node->io_watcher));
			x_printf(D, "start to send back data!\n");
		}
#endif
#ifdef USE_REDIS_PROTOCOL
		if ( p_porter->task.origin == BIT8_TASK_ORIGIN_REDIS ){
			struct data_node *p_node = get_pool_addr( p_porter->task.sfd );
			/*check error*/
			if (err) {
				p_node->control = X_EXECUTE_ERROR;
			}else{
				p_node->control = X_DONE_OK;
			}
			cache_free( &p_node->recv );
			ev_io_init(&(p_node->io_watcher), _swift_send_cb, p_node->sfd, EV_WRITE);
			ev_io_start(p_porter->loop, &(p_node->io_watcher));
			x_printf(D, "start to send back data!\n");
		}
#endif
	}
	return;
}

/*
 * 函 数:swift_all_task_hit
 * 功 能:向所有的swift线程分配任务
 * 参 数:task指向的任务会内存拷贝到porter线程的队列中
 * 指向任务,synch是否同步，如果同步的话该函数直到所有porter线程处理完毕才返回
 * 返回值:
 * 说 明: 
 */
void swift_all_task_hit( struct swift_task_node *task, bool synch, int mark )
{
	int i = 0;
	int id = 0;
	int aband = false;
	char deal = TASK_IS_BEGIN;

	if ( (task->origin == BIT8_TASK_ORIGIN_HTTP) || (task->origin == BIT8_TASK_ORIGIN_REDIS) ){
		/*otherwise, it will depth spiral in _swift_recv_cb.*/
		synch = false;
		aband = true;
	}
	task->type	= BIT8_TASK_TYPE_WHOLE;
	task->deal	= synch ? &deal : NULL;
	do {
		//为任务分配在全局任务中的下标
		id = swift_task_rgst(task->origin, task->type, G_PORTER_COUNTS, G_PORTER_COUNTS, mark);
		if ( id < 0 ) {
			if ( aband ){
				x_printf(S, "ERROR: Too much manage task not done origin from %c, FAILED!\n", task->origin);
				return;//TODO
			}else{
				x_printf(I, "WARNING: Too much manage task not done origin from %c!\n", task->origin);
				sched_yield();//usleep(0);
			}
		}else {
			task->id = id;

			for (i = 0; i < G_PORTER_COUNTS; ++i) {
				x_printf(D, "porter id %d\n", i);
				PORTER_PTHREAD *p_porter = &g_porter_pthread[ i ];
				//把任务通过数组实现的无锁队列交给porter线程
				while ( false == supex_task_push( &(p_porter->tlist), task ) ){
					if ( aband ){
						x_printf(S, "ERROR: push task %d into porter %d list failed. origin from %c!\n", id, i, task->origin);
						break;//TODO use create pthread to swift_one_task_hit add task.
					}else{
						x_printf(I, "WARNING: wait to push task into list origin from %c!\n", task->origin);
						sched_yield();//usleep(0);
					}
				};
			}
		}
	} while( id < 0 );
	//任务处理完之后再返回
	if (synch && task->func){
		while( deal != TASK_IS_FINISH ){
			sched_yield();//usleep(0);
		}
	}
}

void swift_one_task_hit( struct swift_task_node *task, bool synch, int mark )
{
	int id = 0;
	char deal = TASK_IS_BEGIN;
	static unsigned int robin = 0;
	unsigned int step = __sync_fetch_and_add( &robin, 1 ) % G_PORTER_COUNTS;

	if ( (task->origin == BIT8_TASK_ORIGIN_HTTP) || (task->origin == BIT8_TASK_ORIGIN_REDIS) ){
		return;/*don't use in _swift_recv_cb.*/
	}
	task->type	= BIT8_TASK_TYPE_ALONE;
	task->deal	= synch ? &deal : NULL;
	do {
		id = swift_task_rgst(task->origin, task->type, G_PORTER_COUNTS, G_PORTER_COUNTS, mark);
		if ( id < 0 ) {
			x_printf(I, "WARNING: Too much manage task not done origin from %c!\n", task->origin);
			sched_yield();//usleep(0);
		}else {
			task->id = id;

			x_printf(D, "porter id %d\n", step);
			PORTER_PTHREAD *p_porter = &g_porter_pthread[ step ];
			while ( false == supex_task_push( &(p_porter->tlist), task ) ){
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

/*
 * 函 数:swift_one_task_run
 * 功 能:直接在porter线程中处理一个任务
 * 参 数:
 * 返回值:
 * 说 明:
 */

static void swift_one_task_run( PORTER_PTHREAD *p_porter )
{
	struct swift_task_node *p_task = &p_porter->task;
	if ( (p_task->origin != BIT8_TASK_ORIGIN_HTTP) && (p_task->origin != BIT8_TASK_ORIGIN_REDIS) ){
		return;/*only use in _swift_recv_cb.*/
	}
	p_task->type	= BIT8_TASK_TYPE_ALONE;
	p_task->deal	= NULL;
	//对于ORIGIN为ORIGIN_HTTP和ORIGIN_REDIS的task，其p_task->id为p_task->sfd
	p_task->id = swift_task_rgst(p_task->origin, p_task->type, G_PORTER_COUNTS, G_PORTER_COUNTS, p_task->sfd);
	assert( p_task->id == p_task->sfd );
	
	//直接在porter线程中处理任务
	task_handle( p_porter );
}

/*
 *函 数:swift_accept_cb
 *功 能:主线程io的回调函数
 */
void swift_accept_cb(struct ev_loop *loop, ev_io *w, int revents)
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
	//取出一个槽位
	struct addr_node *p_addr_node = mapping_addr_node( newfd );
	//设置槽位的端口号
	p_addr_node->port = ntohs(sin.sin_port);
	memset(p_addr_node->szAddr, 0, INET_ADDRSTRLEN);
	//设置槽位的ＩＰ地址
	inet_ntop(AF_INET, &sin.sin_addr, p_addr_node->szAddr, INET_ADDRSTRLEN);

	swift_dispatch_task(newfd);

}



void swift_prepare_cb(struct ev_loop *loop, ev_prepare *w, int revents)
{
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)(w->data);

	//x_printf(D, "in prepare callback \n");
#ifdef FREE_LOCK_QUEUE
	while ( 1 ){
		if ( false == supex_task_pull(&p_porter->tlist, &p_porter->task) ){
			break;
		}else{
			task_handle(p_porter);
		}
	}
#endif
}

#define MAX_IDLE_SLEEP_DEPTH		5
extern int g_swift_init_done;
void swift_idle_cb(struct ev_loop *loop, ev_idle *w, int revents)
{
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)(w->data);

	if (MAX_IDLE_SLEEP_DEPTH <= __sync_add_and_fetch(&p_porter->depth, 1)){
		__sync_lock_release(&p_porter->depth);
		if (g_swift_init_done && g_swift_settings.conf->vmsys_idle){
			g_swift_settings.conf->vmsys_idle(p_porter);
		}
		usleep(5000);
	}
}


/*
 *函 数:fetch_link_work_task
 *功 能:在异步事件的回调函数中会调用该函数(用来从队列中取出一个新的CQ_ITEM，里面有新的fd,为fd创建data_node,并初始化data_node之后，由porter线程进行监听)
 */
static int fetch_link_work_task(PORTER_PTHREAD *p_porter, struct ev_loop *loop)
{
	//从队列中取出新的CQ_ITEM
	CQ_ITEM *item = cq_pop( &(p_porter->qlist) );

	if (item != NULL) {
		int sfd = item->data;
		struct addr_node *p_addr_node = mapping_addr_node( sfd );
		if( __sync_bool_compare_and_swap(&p_addr_node->work_status, NO_WORKING, IS_WORKING) ){
			struct data_node *p_node = get_pool_addr( sfd );
			p_node->io_watcher.data = p_porter;
			ev_io_init( &(p_node->io_watcher), _swift_recv_cb, p_node->sfd, EV_READ);
			ev_io_start( loop, &(p_node->io_watcher) );
			// x_printf(D, "thread[%lu] accept: fd :%d  addr:%s port:%d\n", p_porter->thread_id, p_node->sfd, p_node->szAddr, p_node->port);
#ifdef OPEN_TIME_OUT
			// x_printf(D, "start timer!\n");
			ev_timer *p_timer = &(p_node->timer_watcher);;
			ev_timer_init(p_timer, _swift_timeout_cb, 5.5, 0. );
			ev_timer_start(loop, p_timer);
#endif
			item_init(item);
			return 1;
		}
		else {
			cq_push( &(p_porter->qlist), item );
			return 0;
		}
	}
	return 0;
}

#ifdef USE_PIPE
/*
 *函 数:fetch_pipe_work_task
 *功 能:从管道中取出一个fd,为该用户创建data_node结点信息，并监听该结点
 *
 */
static int fetch_pipe_work_task(PORTER_PTHREAD *p_porter, struct ev_loop *loop)
{
	int sfd = 0;
	//从管道中取出fd
	int n = read(p_porter->pfds[0], (char *)&sfd, sizeof(int));
	assert( (n == -1) || (n == sizeof(int)) );
	if (n == sizeof(int)){
		//找到fd对应的槽位
		struct addr_node *p_addr_node = mapping_addr_node( sfd );
		if( __sync_bool_compare_and_swap(&p_addr_node->work_status, NO_WORKING, IS_WORKING) ){
			//get_pool_addr为用户创建data_node,初始化之后返回
			struct data_node *p_node = get_pool_addr( sfd );
			//data_node中有io事件，timer事件和用户的收发缓存区
			//设置io事件的附加数据为porter
			p_node->io_watcher.data = p_porter;
			//初始化io事件
			ev_io_init( &(p_node->io_watcher), _swift_recv_cb, p_node->sfd, EV_READ);
			ev_io_start( loop, &(p_node->io_watcher) );
			// x_printf(D, "thread[%lu] accept: fd :%d  addr:%s port:%d\n", p_porter->thread_id, p_node->sfd, p_node->szAddr, p_node->port);
#ifdef OPEN_TIME_OUT
			// x_printf(D, "start timer!\n");
			// 初始化timer事件
			ev_timer *p_timer = &(p_node->timer_watcher);;
			ev_timer_init(p_timer, _swift_timeout_cb, 5.5, 0. );
			ev_timer_start(loop, p_timer);
#endif
			return 1;
		}
		else {
			cq_push( &(p_porter->qlist), &(p_addr_node->recv_item) );
			return 0;
		}
	}
	return 0;
}

#define MAX_TASK_FETCH_DEPTH		25
/*
 *函 数:swift_fetch_cb
 *功 能:porter线程的管道的回调函数(主线程通过管理把fd交给porter线程)
 *参 数:
 *返回值:
 *说 明:
 */
void swift_fetch_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)(w->data);
#if 0
	fetch_pipe_work_task(p_porter, loop);
#else
	int i = 0;
	while( !!fetch_pipe_work_task(p_porter, loop) && (++i <= MAX_TASK_FETCH_DEPTH) ){};
#endif
}

void swift_check_cb(struct ev_loop *loop, ev_check *w, int revents)
{
	int idx = 0;
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)(w->data);
	//从队列中取出一个fd
	while( !!fetch_link_work_task(p_porter, loop) ){ 
		idx++;
	}
	if (idx == 0){
		//从管道中取出一个fd
		fetch_pipe_work_task(p_porter, loop);
	}
}
#else
/*
 * 函 数:swift_async_cb
 * 功 能:
porter　异步事件的回调函数,主要由主线程负责触发，以便通知porter线程从队列中取出一个fd
 * 参数:
 *返回值:
 *说 明:
 */
void swift_async_cb(struct ev_loop *loop, ev_async *w, int revents)
{
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)(w->data);
	fetch_link_work_task(p_porter, loop);
}

void swift_check_cb(struct ev_loop *loop, ev_check *w, int revents)
{
	PORTER_PTHREAD *p_porter = (PORTER_PTHREAD *)(w->data);
	//从队列中取出fd
	while( !!fetch_link_work_task(p_porter, loop) ){ }
}
#endif


/*
 *函 数:_swift_recv_cb
 *功 能:用户io的回调函数,用户io事件由porter线程负责时行监听
 */
static void _swift_recv_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	//获取data_node结点
	struct data_node *p_node = get_pool_addr( w->fd );
	PORTER_PTHREAD *p_porter = w->data;
	//fetch_pipe_work_task(p_porter, loop);
	struct swift_task_node *p_task = &p_porter->task;
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
			struct api_list *p_api = swift_http_api_lookup( p_node->recv.buf_addr + p_node->http_info.hs.path_offset,
					p_node->http_info.hs.path_offset );
			if ( p_api == NULL){
				http_response(p_node, 400);
				p_node->control = X_REQUEST_ERROR;
				break;
			}

			/*make api task*/
			p_task->id	= sfd;
			p_task->sfd	= sfd;
			p_task->type	= p_api->type;
			p_task->origin	= BIT8_TASK_ORIGIN_HTTP;
			p_task->func	= p_api->func;
			p_task->index	= 0;
			p_task->data	= NULL;
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
			//进行数据解析
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

			struct cmd_list *p_cmd = swift_redis_cmd_lookup( p_node->redis_info.rs.order );
			if ( p_cmd == NULL){
				p_node->control = X_INTERIOR_ERROR;
				cache_add( &p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR) );
				break;
			}
			/*make cmd task*/
			p_task->id	= sfd;
			p_task->sfd	= sfd;
			p_task->type	= p_cmd->type;
			p_task->origin	= BIT8_TASK_ORIGIN_REDIS;
			p_task->func	= p_cmd->func;
			p_task->index	= 0;
			p_task->data	= NULL;
			/*reset parse*/
			rds_reset_parser(p_node);
#endif
			/*
			 * can only restart recv until this task done,
			 * should add stop code before task start,
			 * so can't add after task done,because we may use asynchronous mechanism when done whole task.
			 */
			ev_io_stop(loop,  w);
			
			switch ( p_task->type ){
				case BIT8_TASK_TYPE_ALONE:
					//直接在本线程中处理一个任务
					swift_one_task_run( p_porter );
					/* it will quickly run into  _swift_send_cb() when socket is not broken */
					break;
				case BIT8_TASK_TYPE_WHOLE:
					swift_all_task_hit( p_task, false, sfd );
					break;
				default:
					x_printf(D, "Error task type %c\n", p_task->type);
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
	ev_io_init(w, _swift_send_cb, sfd, EV_WRITE);
	ev_io_start(loop, w);
	/* it will quickly run into  _swift_send_cb() when socket is not broken */
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



static void _swift_send_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	int sfd = w->fd;
	x_out_time(&g_dbg_time);

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
	ev_io_init(w, _swift_recv_cb, sfd, EV_READ);
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

static void _swift_timeout_cb(struct ev_loop *loop, ev_timer *w, int revents)
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
 *函 数:swift_update_cb
 *功 能:主线程timer的回调函数　
 */
void swift_update_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	int space = get_overplus_time();
	if (0 == space){
		x_printf(S, "update ...\n");
		/*update c logfd*/
		open_new_log();
		/*update lua logfd*/
		struct swift_task_node task = {
			.id	= 0,
			.sfd	= 0,
			.type	= BIT8_TASK_TYPE_WHOLE,
			.origin	= BIT8_TASK_ORIGIN_TIME,
			.func	= g_swift_settings.conf->vmsys_rfsh,
			.index	= 0,
			.data	= NULL
		};
		swift_all_task_hit( &task, false, 0 );

		/*reget timer*/
		space = get_overplus_time();
	}
	/*reset timer*/
	ev_timer_stop( loop, w );
	ev_timer_init( w, swift_update_cb, space, 0. );
	ev_timer_start( loop, w );
}

void swift_signal_cb (struct ev_loop *loop, ev_signal *w, int revents)
{
	x_printf(D, "get a signal\n");
	if(w->signum == SIGQUIT){
		//TODO free pool buff
		ev_signal_stop( loop, w );
		ev_break (loop, EVBREAK_ALL);
	}
}
