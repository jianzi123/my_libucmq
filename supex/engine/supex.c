#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

#include "supex.h"

static inline unsigned int supply_idle_fetch( struct supex_coro *coro )
{
	int i;
	unsigned int offset;
	bool ok = false;
	for (offset = coro->nxt, i = 0; i < coro->num;
			offset = (offset + 1) % coro->num, i++){
		if (!coroutine_useable( &coro->C[ offset ] )){
			ok = true;
			break;
		}
	}
	assert( ok );
	coro->nxt = (offset + 1) % coro->num;
	return offset;
}

static void *supply_coro_queue(struct schedule *S, void *self)
{
	int i = 0;
	unsigned int idx = 0;
	struct coroutine *H = NULL;
	struct coroutine *L = NULL;
	struct coroutine *C = NULL;

	struct supex_coro *coro = self;


	while ( i++ < CORO_QUEUE_ADD_STEP ){
		if ( !coro->have ){
			if ( false == coro->task_lookup(coro->data, coro->temp) )
				break;
		}
		if ( coro->safe_filter && coro->safe_filter(coro->data, coro->temp) ){
			coro->have = true;
			return H;
		}
		coro->have = false;

		idx = supply_idle_fetch( coro );
		C = &coro->C[ idx ];
		if (!H){
			H = L = C;
		}
		else{
			L->next = C;
			L = C;
		}
		assert( 0 == coroutine_reuse(C) );
		memcpy( &((char *)coro->task)[ idx * coro->tsz ], (char *)coro->temp, coro->tsz );
	}
	return H;
}


void supex_coro_loop(struct supex_coro *coro)
{
	assert( coro && coro->num && coro->tsz && coro->task_lookup && coro->task_handle );
	if (NULL == coro->task){
		coro->task = (void *)calloc( coro->num + 1, coro->tsz );
		assert( coro->task );
		coro->temp = (char *)coro->task + coro->tsz * coro->num;
	}
	if (NULL == coro->VMS){
		coro->VMS = (union virtual_system *)calloc( coro->num, sizeof(union virtual_system) );
		assert( coro->VMS );
	}
	if (NULL == coro->C){
		coro->C = (struct coroutine *)calloc( coro->num, sizeof(struct coroutine) );
		assert( coro->C );
	}
	if (NULL == coro->task_supply){
		coro->task_supply = supply_coro_queue;
	}

	int i = 0;
	for (i = 0; i < coro->num; i++) {
		coroutine_init(&coro->S, &coro->C[i], coro->task_handle, (void *)((uintptr_t)i));
	}
	coroutine_open(&coro->S, coro->task_supply, (void *)coro);
	coroutine_loop(&coro->S);
	coroutine_close(&coro->S);
	return;
}



/*
 * 函 数:supex_line_loop 
 * 功 能:从line中取出一个任务进行处理
 */
void supex_line_loop(struct supex_line *line)
{
	assert( line && line->tsz && line->task_lookup && line->task_handle );
	if (NULL == line->task){
		line->task = (void *)calloc( 1, line->tsz );
		assert( line->task );
	}
	while ( 1 ){
		//从用户的用数组实现的无锁队列tlist中取出一个任务，保存到task中
		if ( false == line->task_lookup(line->data, line->task) ){
			/*
			 *  if use sched_yield();
			 *  it's correct,but may be not good.
			 *  it's substantial increase load average!
			 */
			usleep(5000);
		}else{
			line->task_handle(line->data, line->task);
		}
	}
	return;
}

static void _async_cb(struct ev_loop *loop, ev_async *w, int revents)
{
	bool ok = false;
	struct supex_evuv *evuv = (struct supex_evuv *)(w->data);

	ok = evuv->task_lookup(evuv->data, evuv->task);
	if (ok) {
		x_printf(D, "accept task in _async_cb\n");
		evuv->task_handle(evuv->data, evuv->task);
	}
}
#define MAX_IDLE_SLEEP_DEPTH		5
static void _idle_cb(struct ev_loop *loop, ev_idle *w, int revents)
{
	bool ok = false;
	struct supex_evuv *evuv = (struct supex_evuv *)(w->data);

	ok = evuv->task_lookup(evuv->data, evuv->task);
	if (ok) {
		x_printf(D, "accept task in _idle_cb\n");
		evuv->task_handle(evuv->data, evuv->task);

		__sync_lock_release(&evuv->depth);
		return;
	}

	if (MAX_IDLE_SLEEP_DEPTH <= __sync_add_and_fetch(&evuv->depth, 1)){
		__sync_lock_release(&evuv->depth);
		usleep(5000);
	}
}
#if 0
static void _periodic_cb(struct ev_loop *loop, ev_periodic *w, int revents)
{
}
static void _check_cb(struct ev_loop *loop, ev_check *w, int revents)
{
#if 0
	bool ok = false;
	int idx = 0;
	struct supex_evuv *evuv = w->data;
		//printf("0000000000\n");
	do {
		//printf("1111111111\n");
		ok = evuv->task_lookup(evuv->data, evuv->task);
		if (ok) {
			evuv->task_handle(evuv->data, evuv->task);
		}
	}while(ok && ++idx < 5);
		//printf("2222222222\n");
#endif
}
#endif

void supex_evuv_wake(struct supex_evuv *evuv)
{
#if 0
	while ( ev_async_pending(&(evuv->async_watcher)) ) {
		x_printf(D, "IN ACCEPT_LOOP!\n");
	}
#else
	//if ( ev_async_pending(&(evuv->async_watcher)) ) {
	//	return;
	//}
#endif

	ev_async_send(evuv->loop, &(evuv->async_watcher));
}

void supex_evuv_loop(struct supex_evuv *evuv)
{
	assert( evuv && evuv->tsz && evuv->task_lookup && evuv->task_handle );
	if (NULL == evuv->task){
		evuv->task = (void *)calloc( 1, evuv->tsz );
		assert( evuv->task );
	}
	if (NULL == evuv->loop){
		evuv->loop = ev_loop_new (EVBACKEND_EPOLL | EVFLAG_NOENV);
	}

	evuv->async_watcher.data = evuv;
	ev_async_init( &(evuv->async_watcher), _async_cb);
	ev_async_start( evuv->loop, &(evuv->async_watcher) );

	evuv->idle_watcher.data = evuv;
	ev_idle_init( &(evuv->idle_watcher), _idle_cb);
	ev_idle_start( evuv->loop, &(evuv->idle_watcher) );
#if 0
	evuv->check_watcher.data = evuv;
	ev_check_init( &(evuv->check_watcher), _check_cb);
	ev_check_start( evuv->loop, &(evuv->check_watcher) );

	evuv->periodic_watcher.data = evuv;
	ev_periodic_init( &(evuv->periodic_watcher), _periodic_cb, 0., 10., 0 );
	ev_periodic_start( evuv->loop, &(evuv->periodic_watcher) );
#endif
	ev_loop(evuv->loop, 0);
	ev_loop_destroy(evuv->loop);
	return;
}





/*
 *函 数:supex_task_init
 *功 能:初始化list，list中每个元素大小是tsz, 最大元素个数为max
 *参 数:
 *返回值:
 *说 明:用数组来实现单生产者和单消息之间的无锁队列
 */
void supex_task_init(struct supex_task_list *list, unsigned int tsz, unsigned int max)
{
	memset(list, 0, sizeof(struct supex_task_list));
	list->max = max;
	list->tsz = tsz;
	list->isz = 0;
	list->osz = 0;
	list->slots = calloc( max, tsz );
	assert(list->slots);

	list->head = 0;
	list->tail = 0;
	list->w_lock = ATOMIC_UNLOCK;
	list->r_lock = ATOMIC_UNLOCK;
	return;
}
/*free lock is just read and write don't need lock*/
bool supex_task_push(struct supex_task_list *list, void *task)
{
	bool ok = false;
	assert( list && task );

	while ( ! __sync_bool_compare_and_swap(&list->w_lock, ATOMIC_UNLOCK, ATOMIC_INLOCK) ) {};

	unsigned int next = (list->tail + 1) % list->max;
	if ( list->head != next ){
		memcpy( &((char *)list->slots)[ list->tail * list->tsz ], (char *)task, list->tsz );
		list->tail = next;
		list->isz ++;
		ok = true;
	}
	
	assert ( __sync_bool_compare_and_swap(&list->w_lock, ATOMIC_INLOCK, ATOMIC_UNLOCK) );
	return ok;
}

bool supex_task_pull(struct supex_task_list *list, void *task)
{
	bool ok = false;
	assert( list && task );

	while ( ! __sync_bool_compare_and_swap(&list->r_lock, ATOMIC_UNLOCK, ATOMIC_INLOCK) ) {};

	if(list->head != list->tail) {
		memcpy( task, &((char *)list->slots)[ list->head * list->tsz ], list->tsz );
		list->head = (list->head + 1) % list->max;
		list->osz ++;
		ok = true;
	}

	assert ( __sync_bool_compare_and_swap(&list->r_lock, ATOMIC_INLOCK, ATOMIC_UNLOCK) );
	return ok;
}
