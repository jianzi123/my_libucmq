#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crzpt_plan.h"
#include "crzpt_api.h"


static X_NEW_LOCK g_plan_lock;
static struct crzpt_time_list g_crzpt_time_list = {0};


void crzpt_plan_list_init(void)
{
	X_LOCK_INIT( &g_plan_lock );

	memset(&g_crzpt_time_list, 0, sizeof(struct crzpt_time_list));

	int now = get_current_time();
	crzpt_set_start_time( now );
	x_printf(D, "reset time by %d\n", now);
}

static struct crzpt_time_node *find_time(int time)
{
	X_LOCK( &g_plan_lock );
	struct crzpt_time_node *p_node = (&g_crzpt_time_list.tm_head)->next;
	while(p_node && p_node->time <= time){
		if (p_node->time == time){
			X_UNLOCK( &g_plan_lock );
			return p_node;
		}
		p_node = p_node->next;
	}
	X_UNLOCK( &g_plan_lock );
	return NULL;
}

static void add_time(struct crzpt_time_node *add)
{
	X_LOCK( &g_plan_lock );
	struct crzpt_time_node *p_node = &g_crzpt_time_list.tm_head;
	while(p_node && p_node->next){
		if ( (p_node->time < add->time) && (p_node->next->time > add->time) ){
			add->next = p_node->next;
			add->prev = p_node;
			p_node->next->prev = add;
			p_node->next = add;
			X_UNLOCK( &g_plan_lock );
			return;
		}
		p_node = p_node->next;
	}
	p_node->next = add;
	add->prev = p_node;
	X_UNLOCK( &g_plan_lock );
	return;
}
static struct crzpt_time_node *new_time(int time)
{
	struct crzpt_time_node *p_new = malloc( sizeof(struct crzpt_time_node) );
	if(p_new == NULL){
		x_perror("new time");
		return NULL;
	}
	memset(p_new, 0, sizeof(struct crzpt_time_node));
	p_new->time = time;
	p_new->pl_head.next = &p_new->pl_head;
	p_new->pl_head.prev = &p_new->pl_head;
	return p_new;
}

void crzpt_set_start_time(int start_time)
{
	X_LOCK( &g_plan_lock );
	struct crzpt_time_node *p_node = NULL;
	g_crzpt_time_list.done = &g_crzpt_time_list.tm_head;
	for( p_node = g_crzpt_time_list.done->next; p_node && (p_node->time < start_time); p_node = p_node->next ){
		x_printf(D, ">>>>>>>>>>>>>>>>>>%d %d\n", p_node->time, start_time);
		g_crzpt_time_list.done = g_crzpt_time_list.done->next;
	}
	x_printf(D, "%p\n", g_crzpt_time_list.done);
	X_UNLOCK( &g_plan_lock );
}

int crzpt_get_next_time(int start_time)
{
	X_LOCK( &g_plan_lock );
	struct crzpt_time_node *p_node = g_crzpt_time_list.done->next;
	if(p_node && (p_node->time > start_time) ){
		x_printf(D, "now time is %d | plan time is %d\n", start_time, p_node->time);
		X_UNLOCK( &g_plan_lock );
		return (p_node->time - start_time);
	}
	x_printf(D, "now time is %d | over time is %d\n", start_time, ONE_DAY_TIMESTAMP);
	int delay = ONE_DAY_TIMESTAMP - start_time;
	X_UNLOCK( &g_plan_lock );
	return ((delay > 0) ? delay : 1);
}

/********************************TASK**************************************/
struct crzpt_time_node *crzpt_get_plan_list(int end_time)
{
	X_LOCK( &g_plan_lock );
	struct crzpt_time_node *p_node = g_crzpt_time_list.done->next;
	if(p_node && p_node->time <= end_time){
		g_crzpt_time_list.done = p_node;
		x_printf(D, "<<<<<<<<<<<<<<<<<<<\n");
		X_UNLOCK( &g_plan_lock );
		return p_node;
	}
	X_UNLOCK( &g_plan_lock );
	return NULL;
}


extern struct crzpt_settings g_crzpt_settings;
void *crzpt_cpp_make_plan(int time, short live, char *data)
{
	static unsigned int g_pidx_mark = 0;
	unsigned int pidx = __sync_fetch_and_add(&g_pidx_mark, 1);

	x_printf(D, "pidx %d time %d live %d\n", pidx, time, live);
	
	if ((time >= ONE_DAY_TIMESTAMP) || ((live != 0) && (live != 1)) ){
		x_perror("error plan parameter!");
		goto FAIL_NEW_PLAN;
	}
	/*ok*/
	struct crzpt_time_node *p_time = NULL;
	struct crzpt_plan_node *p_plan = NULL;
	/*new*/
	p_plan = calloc( 1, sizeof(struct crzpt_plan_node) );
	if (p_plan == NULL){
		x_perror("new plan");
		goto FAIL_NEW_PLAN;
	}
	/*set*/
	p_plan->pidx = pidx;
	p_plan->live = live;
	/*add*/
	p_time = find_time( time );
	if (p_time == NULL){
		p_time = new_time( time );
		if (p_time == NULL){
			free(p_plan);
			x_perror("new time");
			goto FAIL_NEW_PLAN;
		}
		add_time(p_time);
	}

	X_LOCK( &g_plan_lock );
	p_time->nubs++;
	p_plan->next = &p_time->pl_head;
	p_plan->prev = p_time->pl_head.prev;
	p_time->pl_head.prev->next = p_plan;
	p_time->pl_head.prev = p_plan;

	char temp[64] = {0};
	sprintf(temp, "%d", pidx);
	g_crzpt_settings.conf->store_insert(temp, strlen(temp), data, strlen(data));//FIXME
	X_UNLOCK( &g_plan_lock );

	return p_plan;
FAIL_NEW_PLAN:
	return NULL;
}


void crzpt_list_for_each(struct crzpt_time_node *p_list, PLAY_FCB play_fcb, struct crzpt_task_node *p_task)
{
#if 0
	X_LOCK( &g_plan_lock );

	int nubs = p_list->nubs;
	struct crzpt_plan_node *p_play = NULL;
	struct crzpt_plan_node *p_plan = p_list->pl_head.next;
	x_printf(D, "nubs = %d, plan= %p, head = %p\n", nubs, p_plan, &p_list->pl_head);

	for( ; (nubs > 0) && (p_plan != NULL) && (p_plan != &p_list->pl_head); nubs-- ){
		p_play = p_plan;
		p_plan = p_plan->next;
		/*must remove first*/
		if (p_play->live == EXECUTE_DEATH){
			p_list->nubs--;
			p_play->next->prev = p_play->prev;
			p_play->prev->next = p_play->next;
			p_play->prev = NULL;
			p_play->next = NULL;
		}
		play_fcb(p_play, p_task);
	}
	X_UNLOCK( &g_plan_lock );
#else
	/*This method we can add plan when the list at run*/
	struct crzpt_plan_node *p_play = NULL;
	struct crzpt_plan_node *p_plan = p_list->pl_head.next;

	while( (p_plan != NULL) && (p_plan != &p_list->pl_head) ){
		X_LOCK( &g_plan_lock );

		p_play = p_plan;
		p_plan = p_plan->next;
		/*must remove first*/
		if (p_play->live == EXECUTE_DEATH){
			p_list->nubs--;
			p_play->next->prev = p_play->prev;
			p_play->prev->next = p_play->next;
			p_play->prev = NULL;
			p_play->next = NULL;
		}
		X_UNLOCK( &g_plan_lock );

		play_fcb(p_play, p_task);
	}
#endif
}

void crzpt_free_all_plan(void)
{
	struct crzpt_time_node *p_time = NULL;
	struct crzpt_time_node *p_next = NULL;
	struct crzpt_plan_node *p_plan = NULL;
	struct crzpt_plan_node *p_temp = NULL;

	X_LOCK( &g_plan_lock );
	for(p_time = (&g_crzpt_time_list.tm_head)->next; p_time; p_time = p_next){
		for(p_plan = (&p_time->pl_head)->next; p_plan && (p_plan != &p_time->pl_head); p_plan = p_temp){
			p_temp = p_plan->next;
			free(p_plan);
		}
		p_next = p_time->next;
		free(p_time);
	}
	memset(&g_crzpt_time_list, 0, sizeof(struct crzpt_time_list));
	X_UNLOCK( &g_plan_lock );
	return;
}
