/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file   kv_timer.c
 * @brief  libkv timer mechanism implementation.
 * 
 * @author shishengjie
 * @date   2015-05-28
 */

#include "fmacros.h"
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "kv_timer.h"
#include "zmalloc.h"
#include <assert.h>


void logicErrorExpr(int expr, const char* fmt, ...);
long long ustime();


int kv_timer_state(kv_timer_t *timer)
{
        return timer->state;
}

static void* _timer_instance(void *arg)
{
        int fatal;
        static int fatal_count = 0;
        struct timeval timeout;
        kv_timer_event_t *ev;
        kv_timer_t *timer = (kv_timer_t*)arg;

        assert(timer->state == KV_TIMER_STATE_LOADING);
        timer->state = KV_TIMER_STATE_RUNNING;
        for (;;) {
                timeout.tv_sec = 0;
                timeout.tv_usec = timer->interval;
                /** interval is very short, ignore signal interrupt */
                fatal = select(0, NULL, NULL, NULL, &timeout);
                if (fatal == -1) {
                        fatal_count++;
                        if (fatal_count >= 3) abort();
                        continue;
                }
                fatal_count = 0;

                ev = timer->list;
                while(ev) {
                        if (ustime() - ev->elapse_base >= ev->timeout) {
                                ev->elapse_base = ustime(); /** restart */
                                ev->proc();
                        }
                        ev = ev->next;
                }
        }
}

static int _timer_start(kv_timer_t *timer)
{
        int ret;
        
        if (kv_timer_state(timer) != KV_TIMER_STATE_STOP) {
                return 1;
        } else {
                timer->state = KV_TIMER_STATE_LOADING;
                ret = pthread_create(&timer->self, NULL, _timer_instance, timer);
                return !ret;
        }
}

static int _timer_stop(kv_timer_t *timer)
{
        struct timespec spec;
        
        if (kv_timer_state(timer) == KV_TIMER_STATE_STOP) {
                return 1;
        } else {
                
                while(kv_timer_state(timer) != KV_TIMER_STATE_RUNNING) {
                        spec.tv_sec = 0;
                        spec.tv_nsec = 10;
                        nanosleep(&spec, NULL);
                }
                pthread_cancel(timer->self);
                pthread_join(timer->self, NULL);
                timer->self = 0;
                timer->state = KV_TIMER_STATE_STOP;
        }

        return 1;
}

kv_timer_t* kv_timer_create(long long interval)
{
        kv_timer_t *timer;

        timer = zmalloc(sizeof(*timer));
        if (!timer) return timer;
        timer->self = 0;
        timer->state = KV_TIMER_STATE_STOP;
        timer->list = 0;
        timer->interval = interval;
        return timer;
}

void kv_timer_destroy(kv_timer_t *timer)
{
        kv_timer_event_t *cur, *next;

        if (!timer) return;
        cur = timer->list;
        while(cur) {
                next = cur->next;
                zfree(cur);
                cur = next;
        }
        zfree(timer);
}

void kv_timer_add(kv_timer_t *timer, long long usec, timer_proc proc)
{
        kv_timer_event_t *te;

        te = zmalloc(sizeof(*te));
        te->timeout = usec < 0 ? -usec : usec;
        te->elapse_base = ustime();
        te->proc = proc;
        te->next = 0;
        
        if (!timer->list) {
                timer->list = te;
        } else {
                te->next = timer->list;
                timer->list = te;
        }
}

int kv_timer_start(kv_timer_t *timer)
{
        return _timer_start(timer);
}

int kv_timer_stop(kv_timer_t *timer)
{
        return _timer_stop(timer);
}
