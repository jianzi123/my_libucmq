#pragma once

#include "ev.h"
#include "utils.h"
#include "coro.h"

struct supex_coro {
	unsigned int num;
	unsigned int tsz;
	unsigned int nxt;
	bool have;
	void *data;
	bool (*safe_filter)(void *data, void *addr);
	bool (*task_lookup)(void *data, void *addr);
	void *(*task_supply)(struct schedule *S, void *self);
	void *(*task_handle)(struct schedule *S, void *step);
	void *temp;		/*No.(num + 1)*/
	void *task;		/*num + 1*/
	union virtual_system *VMS;
	struct coroutine *C;
	struct schedule S;
};

void supex_coro_loop(struct supex_coro *coro);


struct supex_line {
	unsigned int tsz;
	void *task;
	void *data;

	union virtual_system VMS;
	bool (*task_lookup)(void *data, void *addr);
	void (*task_handle)(void *data, void *addr);
};

void supex_line_loop(struct supex_line *line);


struct supex_evuv {
	unsigned int tsz;
	void *task;
	void *data;

	union virtual_system VMS;
	bool (*task_lookup)(void *data, void *addr);
	void (*task_handle)(void *data, void *addr);

	unsigned int depth;
	struct ev_loop *loop;
	struct ev_async async_watcher;
	struct ev_idle idle_watcher;
#if 0
	struct ev_periodic periodic_watcher;
	struct ev_check check_watcher;
#endif
};

void supex_evuv_loop(struct supex_evuv *evuv);
void supex_evuv_wake(struct supex_evuv *evuv);




struct supex_task_list {
	unsigned int max;
	unsigned int tsz;
	unsigned int isz;
	unsigned int osz;
	void *slots;

	unsigned int head;
	unsigned int tail;
	int w_lock;
	int r_lock;
};

void supex_task_init(struct supex_task_list *list, unsigned int tsz, unsigned int max);

bool supex_task_push(struct supex_task_list *list, void *task);

bool supex_task_pull(struct supex_task_list *list, void *task);
