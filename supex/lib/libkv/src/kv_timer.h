/**
 * Copyright (c) 2015
 * All Rights Reserved
 *
 * @file   kv_timer.h
 * @brief  libkv timer declaration.
 *
 * @author shishengjie
 * @date   2015-05-28
 */

#ifndef KV_TIMER_H_
#define KV_TIMER_H_




typedef void (*timer_proc)(void);


typedef struct kv_timer_event {
        long long timeout;
        long long elapse_base;
        timer_proc proc;
        struct kv_timer_event *next;
}kv_timer_event_t;


typedef enum kv_timer_state {
        KV_TIMER_STATE_STOP = 0,
        KV_TIMER_STATE_LOADING,
        KV_TIMER_STATE_RUNNING
}kv_timer_state_t;


typedef struct kv_timer {
        kv_timer_event_t *list;
        pthread_t self;
        kv_timer_state_t state;
        long long interval;
}kv_timer_t;




kv_timer_t* kv_timer_create(long long interval);
void kv_timer_destroy(kv_timer_t *timer);
int kv_timer_start(kv_timer_t *timer);
int kv_timer_stop(kv_timer_t *timer);
void kv_timer_add(kv_timer_t *timer, long long usec, timer_proc proc);
int kv_timer_running(kv_timer_t *timer);




#endif
