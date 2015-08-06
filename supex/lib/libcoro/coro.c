#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sched.h>

#include "coro.h"


struct schedule *coroutine_open(struct schedule *S, coroutine_func func, void *data)
{
	if (S){
		S->origin = ORIGIN_FROM_EXTERNAL;
	}else{
		S = calloc(sizeof(*S), 1);
		S->origin = ORIGIN_FROM_INTERNAL;
	}
	assert(S);
	S->nubs = 0;
	S->exit = NULL;
	S->exec = &S->main;
	S->main.prev = &S->main;
	S->main.next = &S->main;
	S->main.func = func;
	S->main.data = data;
#ifdef CORO_USE_LEAST_SPACE
#else
	S->main.safe_mark = CORO_SAFE_SIGNATURE;
#endif
	/*others don't use,because we don't know stack top,so also we don't know stack use size.*/
	return S;
}

void coroutine_close(struct schedule *S)
{
	struct coroutine *T = NULL;
	struct coroutine *C = S->main.next;
	while(C && (C != &S->main)){
#ifdef CORO_USE_LEAST_SPACE
		if(C->stack){
			free(C->stack);
		}
#endif
		T = C->next;
		if (C->origin == ORIGIN_FROM_INTERNAL){
			free(C);
		}
		C = T;
	}
	if (S->origin == ORIGIN_FROM_INTERNAL){
		free(S);
	}
	return;
}
void _push(struct schedule *S, struct coroutine *C)
{
	assert(C && (C->status == COROUTINE_INITIAL));
	C->next = &S->main; 
	C->prev = S->main.prev; 
	S->main.prev->next = C; 
	S->main.prev = C; 
	C->status = COROUTINE_ENABLED;
	++S->nubs;
	return;
}

void _pop(struct schedule *S, struct coroutine *C)
{
	assert(C && (C != &S->main));
	C->next->prev = C->prev; 
	C->prev->next = C->next;
	C->prev = NULL;
	C->next = NULL;
	C->status = COROUTINE_RELEASE;
	/*if (back stack info) or (back ucontext_t info) on C, we can't free C before back main coroutine*/
	S->exit = C;
	--S->nubs;
	return;
}

static int _init(struct schedule *S, struct coroutine *C, int origin, coroutine_func func, void *data)
{
	if (!(C && S))
		return -1;
	C->sch = S;
	/*we should init ucontext_t before swapcontext*/
#ifdef CORO_USE_LEAST_SPACE
	C->ctx.uc_stack.ss_sp = S->stack;
#else
	C->ctx.uc_stack.ss_sp = C->stack;
#endif
	C->ctx.uc_stack.ss_size = STACK_SIZE;
	C->ctx.uc_link = &S->main.ctx;

#ifdef CORO_USE_LEAST_SPACE
	C->stack = NULL;
	C->maxsize = 0;
	C->offsize = 0;
#endif
	C->status = COROUTINE_INITIAL;
	C->origin = origin;

	C->func = func;
	C->data = data;

	C->prev = NULL;
	C->next = NULL;
#ifdef CORO_USE_LEAST_SPACE
#else
	C->safe_mark = CORO_SAFE_SIGNATURE;
#endif
	return 0;
}
int coroutine_reuse(struct coroutine *C)
{
	if (!C)
		return -1;
#ifdef CORO_USE_LEAST_SPACE
	C->stack = NULL;
	C->maxsize = 0;
	C->offsize = 0;
#endif
	C->status = COROUTINE_INITIAL;
	return 0;
}

int coroutine_useable(struct coroutine *C)
{
	if ( C->status == COROUTINE_INITIAL || C->status == COROUTINE_RELEASE )
		return 0;
	else
		return -1;
}

struct coroutine *coroutine_create(struct schedule *S, coroutine_func func, void *data)
{
	struct coroutine *C = malloc(sizeof(*C));
	return ( 0 == _init(S, C, ORIGIN_FROM_INTERNAL, func, data) )? C : NULL;
}

int coroutine_init(struct schedule *S, struct coroutine *C, coroutine_func func, void *data)
{
	return _init(S, C, ORIGIN_FROM_EXTERNAL, func, data);
}

static void _resume(struct schedule *S, struct coroutine *C);

static void _task(uint32_t low32, uint32_t hi32) {
	struct schedule *S = (struct schedule *) ((uintptr_t)low32 | ((uintptr_t)hi32 << 32));
	struct coroutine *C = S->exec;
	C->func(S, C->data);
	S->exec = C->prev;
	_pop(S, C);
	return;
}
/*
 * why don't save stack in yield and back stack in resume when it is in main coroutine,because stack is unknow.
 * main coroutine don't use yield and resume way to keep working.
 */
static void _yield(struct schedule *S, struct coroutine *C)
{
	char dummy = 0;
	C->status = COROUTINE_SUSPEND;
	/*save stack*/
	if (C != &S->main){
#ifdef CORO_USE_LEAST_SPACE
		C->offsize = (S->stack + STACK_SIZE) - &dummy;
		assert(C->offsize <= STACK_SIZE);
		if (C->maxsize < C->offsize) {
			if (C->stack){
				free(C->stack);
			}
			C->maxsize = C->offsize;
			C->stack = malloc(C->maxsize);
			assert(C->stack);
		}
		memcpy(C->stack, &dummy, C->offsize);
#else
#endif
	}
}

static void _resume(struct schedule *S, struct coroutine *C)
{
	if (C != &S->main){
		switch(C->status) {
			case COROUTINE_ENABLED:
				getcontext(&C->ctx);
				makecontext(&C->ctx, (void (*)(void))_task, 2, (uint32_t)((uintptr_t)S), (uint32_t)(((uintptr_t)S)>>32));
				break;
			case COROUTINE_SUSPEND:
#ifdef CORO_USE_LEAST_SPACE
				memcpy(S->stack + STACK_SIZE - C->offsize, C->stack, C->offsize);
#else
#endif
				break;
			default:
				assert(0);
		}
	}
	if (C != &S->main){
		S->exec = C;	/*when C = S->main,we must't set exec = S->main*/
	}
	C->status = COROUTINE_RUNNING;
	return;
}


void coroutine_switch(struct schedule *S)
{
	struct coroutine *Y = S->exec;
#ifdef OPEN_FAST_SWITCH
	struct coroutine *R = S->exec->next;/*if one cycle is over,switch to main coroutine first.*/
#else
	struct coroutine *R = &S->main;
#endif
	assert(R != Y);
#ifdef CORO_USE_LEAST_SPACE
#else
	assert(R->safe_mark == CORO_SAFE_SIGNATURE);
#endif
	_yield(S, Y);
#ifdef OPEN_MAIN_STATUS_CNTL
	_resume(S, R);
#endif
	swapcontext(&Y->ctx , &R->ctx);
	return;
}

/*
 * we should avoid stack leak,this is why always swap to main coroutine then jump to other coroutine.
 */
void coroutine_loop(struct schedule *S)
{
	struct coroutine *C = NULL;
	struct coroutine *L = NULL;
	struct coroutine *R = NULL;
	struct coroutine *Y = &S->main;
	while(1){
		//printf("loop ... .. .\n");
		//printf("%x:%d\n", S->main.ctx.uc_stack.ss_sp, S->main.ctx.uc_stack.ss_size);
		R = S->exec->next;
		assert((R != S->exec) || (R == S->exec && R == Y));
		if (R == Y){
#ifdef OPEN_LESS_SWITCH
			if ((S->append == false) &&
					(S->last == S->nubs)){
					usleep(500);
			}
			S->append = false;
#endif
			while(1){
				if (S->nubs <= CORO_QUEUE_LOW_SLOT){
					L = S->main.func(S, S->main.data);
#ifdef OPEN_LESS_SWITCH
					S->append = (L == NULL)?false:true;
#endif
					while ( C = L){
						L = C->next;/*record next before push*/
						_push(S, C);/*C->next is change*/
					}
				}
				if (S->nubs == 0){
#if 1
					usleep(5000);
#else
					sched_yield();/*it's correct,but may be not good,it's substantial increase load average!*/
#endif
				}
				else{
					R = R->next;
					assert(R != Y);
					break;
				}
			}
#ifdef OPEN_LESS_SWITCH
			S->last = S->nubs;
#endif
		}
#ifdef CORO_USE_LEAST_SPACE
#else
		assert(R->safe_mark == CORO_SAFE_SIGNATURE);
#endif
#ifdef OPEN_MAIN_STATUS_CNTL
		_yield(S, Y);
#endif
		_resume(S, R);
		swapcontext(&Y->ctx , &R->ctx);
		if (S->exit){
#ifdef CORO_USE_LEAST_SPACE
			if (S->exit->stack){
				free(S->exit->stack);
			}
#endif
			if (S->exit->origin == ORIGIN_FROM_INTERNAL){
				free(S->exit);
			}
			S->exit = NULL;
		}
	}
	return;
}
