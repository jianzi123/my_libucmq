#include <stdio.h>
#include <unistd.h>
#include <sched.h>

#include "crzpt_evcb.h"
#include "crzpt_api.h"
#include "crzpt_plan.h"

static int g_roamer_time = -1;				/*for app init first*/

extern int G_ROAMER_COUNTS;
#ifdef OPEN_CORO
extern int G_PAUPER_COUNTS;
extern int G_PAUPER_COUNTS_TIMES;
#endif

extern struct crzpt_settings g_crzpt_settings;
extern FEEDER_PTHREAD g_feeder_pthread;					/* send task */
extern ROAMER_PTHREAD *g_roamer_pthread;				/* do task */

#ifdef CRZPT_OPEN_MSMQ
#else
#if 0
static void crzpt_signal_cb (struct ev_loop *loop, ev_signal *w, int revents)
{
	printf("get a signal\n");
	char *data = NULL;
	/*
	 * 1. in libev, singnal > 32 use as a unreliable
	 * 2. when a singnal come, it week up the sleep thread and add a sinal function in loop but not do it quickly.
	 * 3. when loop run into sinal function, the hide signal callback will be done,so don't care the crzpt_time_node is in use.
	 */
	switch(w->signum) {
		case SIGQUIT:
			//TODO free all list or use
			ev_signal_stop( loop, w );
			ev_break (loop, EVBREAK_ALL);
			break;
		case SIG_APP_ADD:
			data = get_fifo_msg();
			//TODO add modele
			break;
		case SIG_APP_SET:
			data = get_fifo_msg();
			//TODO updata modele
			break;
		case SIG_APP_DEL:
			data = get_fifo_msg();
			//TODO del modele
			break;
		default:
			break;
	}
}
#endif
#endif

void crzpt_all_task_hit( struct crzpt_task_node *task, bool synch )
{
	int i = 0;
	int id = 0;
	char deal = TASK_IS_BEGIN;

	task->type	= BIT8_TASK_TYPE_WHOLE;
	task->deal	= synch ? &deal : NULL;
	do {
#ifdef OPEN_CORO
		/*
		while( is_empty() ){
			sched_yield();//usleep(0);
		}
		*/
		id = crzpt_task_rgst(task->origin, task->type, G_ROAMER_COUNTS, G_PAUPER_COUNTS);
#else
		id = crzpt_task_rgst(task->origin, task->type, G_ROAMER_COUNTS, G_ROAMER_COUNTS);
#endif
		if ( id < 0 ) {
			x_printf(I, "WARNING: Too much manage task not done!\n");
			sched_yield();//usleep(0);
		}else {
			task->id = id;
			for (i = 0; i < G_ROAMER_COUNTS; ++i) {
				x_printf(D, "roamer id %d\n", i);
				while ( false == g_crzpt_settings.conf->task_report( &g_roamer_pthread[ i ], task ) ){
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

void crzpt_one_task_hit( struct crzpt_task_node *task, bool synch )
{
	int id = 0;
	char deal = TASK_IS_BEGIN;
	static unsigned int robin = 0;
	unsigned int idx = __sync_fetch_and_add( &robin, 1 ) % G_ROAMER_COUNTS;

	task->type	= BIT8_TASK_TYPE_ALONE;
	task->deal	= synch ? &deal : NULL;
	do {
#ifdef OPEN_CORO
		/*
		while( is_empty() ){
			sched_yield();//usleep(0);
		}*/
		id = crzpt_task_rgst(task->origin, task->type, G_ROAMER_COUNTS, G_PAUPER_COUNTS);
#else
		id = crzpt_task_rgst(task->origin, task->type, G_ROAMER_COUNTS, G_ROAMER_COUNTS);
#endif
		if ( id < 0 ) {
			x_printf(I, "WARNING: Too much manage task not done!\n");
			sched_yield();//usleep(0);
		}else {
			task->id = id;

			x_printf(D, "roamer id %d\n", idx);
			while ( false == g_crzpt_settings.conf->task_report( &g_roamer_pthread[ idx ], task ) ){
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

static void afresh_system(void)
{
	crzpt_free_all_plan( );
	struct crzpt_task_node task = {};
	task.origin	= BIT8_TASK_ORIGIN_TIME;

	/*exit*/
	task.type	= BIT8_TASK_TYPE_WHOLE;
	task.func	= g_crzpt_settings.conf->vmsys_exit;
	crzpt_all_task_hit( &task, true );
	task.type	= BIT8_TASK_TYPE_WHOLE;
	task.func	= g_crzpt_settings.conf->vmsys_init;
	crzpt_all_task_hit( &task, true );
	task.type	= BIT8_TASK_TYPE_ALONE;
	task.func	= g_crzpt_settings.conf->vmsys_load;
	crzpt_one_task_hit( &task, true );
}

static void _plan_begin_play( struct crzpt_plan_node *p_plan, struct crzpt_task_node *p_task )
{
	/*dispath to a pthread.*/
	p_task->data	= (void *)p_plan;

	crzpt_one_task_hit( p_task, false );
}

void crzpt_idle_cb(struct ev_loop *loop, ev_idle *w, int revents)
{
	int now = 0;
	/*check*/
	now = get_current_time();
	if (now < g_roamer_time){
		afresh_system();

		/* when worker is reset */
		crzpt_set_start_time(0);
	}
	g_roamer_time = now;
	/*start*/
	struct crzpt_time_node *p_list = crzpt_get_plan_list( now );
	if (p_list == NULL){
		x_printf(D, "________1__________\n");
		int delay = crzpt_get_next_time( now );

		int num = msmq_call( );
		if (num > 0){
			x_printf(D, "==%d==\n", num);
			return;
		}
		x_printf(D, "need sleep %d but sleep %d\n", delay, MIN(delay, 5));
		sleep( MIN(delay, 5) );/*make it small*/
	}else{
		x_printf(D, "________2__________\n");
		/*call task*/
		struct crzpt_task_node task = {};
		task.origin	= BIT8_TASK_ORIGIN_TIME;
		task.type	= BIT8_TASK_TYPE_ALONE;
		task.func	= g_crzpt_settings.conf->vmsys_call;

		crzpt_list_for_each(p_list, _plan_begin_play, &task);
	}
}
