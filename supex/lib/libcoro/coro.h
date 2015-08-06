#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <ucontext.h>

#ifndef CORO_USE_STATIC_STACK
#define CORO_USE_LEAST_SPACE
#endif
#define OPEN_FAST_SWITCH
#define OPEN_LESS_SWITCH

#define COROUTINE_INITIAL	0
#define COROUTINE_ENABLED	1
#define COROUTINE_RUNNING	2
#define COROUTINE_SUSPEND	3
#define COROUTINE_RELEASE	4

#define ORIGIN_FROM_EXTERNAL	0
#define ORIGIN_FROM_INTERNAL	1

#define CORO_SAFE_SIGNATURE	0x6a


#ifdef CORO_USE_LEAST_SPACE
#define STACK_SIZE		(1024*1024*16)
#define CORO_QUEUE_ADD_STEP	32
#else
#define STACK_SIZE		(1024*1024*8)
#define CORO_QUEUE_ADD_STEP	2
#endif

#define CORO_QUEUE_LOW_SLOT	(CORO_QUEUE_ADD_STEP * 15)
#define CORO_QUEUE_MIN_SLOT	(CORO_QUEUE_ADD_STEP * 16)

#ifdef OPEN_FAST_SWITCH
#define OPEN_MAIN_STATUS_CNTL
#endif
struct schedule;
typedef void *(*coroutine_func)(struct schedule *S, void *data);

/*
 * safe_mark must add at head and tail,because stack is grow upwards.
 * Of course, we can also add safe_mark at tail and move stack to head.
 */
struct coroutine {
#ifdef CORO_USE_LEAST_SPACE
	char *stack;
	ptrdiff_t maxsize;	/*stack max size*/
	ptrdiff_t offsize;	/*stack use size*/
#else
	char stack[STACK_SIZE];
#endif
	struct schedule *sch;
	ucontext_t ctx;

	int status;
	int origin;
	coroutine_func func;
	void *data;
	
	struct coroutine *prev;
	struct coroutine *next;
#ifdef CORO_USE_LEAST_SPACE
#else
	char safe_mark;
#endif
};

struct schedule {
#ifdef CORO_USE_LEAST_SPACE
	char stack[STACK_SIZE];
#endif
	int nubs;		/*coro count*/
#ifdef OPEN_LESS_SWITCH
	int last;		/*last coro count*/
	bool append;
#endif
	int origin;
	struct coroutine main;
	struct coroutine *exec;	/*exec context*/
	struct coroutine *exit;	/*exit context*/
};


struct schedule *coroutine_open(struct schedule *S, coroutine_func func, void *data);
void coroutine_close(struct schedule *S);

/*===dynamic method===*/
struct coroutine *coroutine_create(struct schedule *S, coroutine_func func, void *data);
/*=======ending=======*/

/*===static method===*/
int coroutine_init(struct schedule *S, struct coroutine *C, coroutine_func func, void *data);
int coroutine_reuse(struct coroutine *C);
/*=======ending=======*/

int coroutine_useable(struct coroutine *C);

void coroutine_switch(struct schedule *S);
void coroutine_loop(struct schedule *S);
