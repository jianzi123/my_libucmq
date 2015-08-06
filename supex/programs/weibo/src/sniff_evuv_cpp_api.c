#include <assert.h>
#include <unistd.h>

#include "sniff_api.h"
#include "sniff_evuv_cpp_api.h"
#include "weibo_cfg.h"
#include "tcp_api.h"
#include "apply_entry.h"


extern struct weibo_cfg_file g_weibo_cfg_file;

int isspace(int x)
{
	return (x==' '||x=='\t'||x=='\n'||x=='\f'||x=='\b'||x=='\r')?1:0;
}
int isdigit(int x)
{
	return (x<='9'&&x>='0')?1:0;

}
int x_atoi(const char *nptr, int size)
{
	int c;              /* current char */
	int total;         /* current total */
	int sign;           /* if '-', then negative, otherwise positive */

	/* skip whitespace */
	while ( size > 0 && isspace((int)(unsigned char)*nptr) ){
		++nptr;
		--size;
	}
	if(size <= 0){
		return 0;
	}

	c = (int)(unsigned char)*nptr++;
	sign = c;           /* save sign indication */
	if (c == '-' || c == '+'){
		c = (int)(unsigned char)*nptr++;    /* skip sign */
		if(--size <= 0){
			return 0;
		}
	}
	total = 0;

	while (size-- > 0 && isdigit(c)) {
		total = 10 * total + (c - '0');     /* accumulate digit */
		c = (int)(unsigned char)*nptr++;    /* get next char */
	}

	return (sign == '-')?-total:total;   /* return result, negated if necessary */
}

static void _vms_erro( void **base )
{
}

static int _vms_cntl( void **base, int last, struct sniff_task_node *task )
{
	struct msg_info *msg = task->data;
	assert( msg );

	switch (msg->opt){
		case 'l':
			break;
		case 'f':
			break;
		case 'o':
			break;
		case 'c':
			break;
		case 'd':
			break;
		default:
			x_printf(S, "Error msmq opt!\n");
			return 0;
	}
	return 0;
}
int sniff_vms_cntl(void *user, void *task)
{
	return sniff_for_alone_vm( user, task, _vms_cntl, _vms_erro );
}


static void *_vms_new( void )
{
	return NULL;
}
static int _vms_init( void **base, int last, struct sniff_task_node *task )
{
	if (*base != NULL){
		x_printf(S, "No need to init LUA VM!\n");
		return 0;
	}
	*base = _vms_new( );
	//assert( *base );
	return 0;
}
int sniff_vms_init(void *user, void *task)
{
	int error = 0;
	error = sniff_for_alone_vm( user, task, _vms_init, _vms_erro );
	if (error) {
		exit(EXIT_FAILURE);
	}
	return error;
}




static int _vms_exit( void **base, int last, struct sniff_task_node *task )
{
	*base = NULL;
	return 0;/*must return 0*/
}

int sniff_vms_exit(void *user, void *task)
{
	int error = sniff_for_alone_vm( user, task, _vms_exit, _vms_erro );
	x_printf(S, "exit one alone LUA!\n");
	return error;
}

static int _vms_rfsh( void **base, int last, struct sniff_task_node *task )
{
	return 0;
}
int sniff_vms_rfsh(void *user, void *task)
{
	return sniff_for_alone_vm( user, task, _vms_rfsh, _vms_erro );
}


static int _vms_sync( void **base, int last, struct sniff_task_node *task )
{
	return 0;
}
int sniff_vms_sync(void *user, void *task)
{
	return sniff_for_alone_vm( user, task, _vms_sync, _vms_erro );
}


static int _vms_gain( void **base, int last, struct sniff_task_node *task )
{
	return 0;
}
int sniff_vms_gain(void *user, void *task)
{
	return sniff_for_alone_vm( user, task, _vms_gain, _vms_erro );
}

int sniff_vms_call(void *user, void *task)
{
	struct sniff_task_node *p_task = task;
	PARTER_PTHREAD *p_parter = (PARTER_PTHREAD *)user;
	time_t delay = time(NULL) - p_task->stamp;

	x_printf(S, "channel %d\t|task <thread> %lu\t<shift> %d\t<come> %lu\t<delay> %lu\n",
			p_parter->genus, p_task->thread_id, p_task->shift, p_task->stamp, delay);
	if (strstr(p_task->data,"groupID")){
		entry_group_weibo( p_parter->evuv.loop, p_task->data, p_task->size );
	}

	if (strstr(p_task->data,"regionCode")){
		entry_city_weibo( p_parter->evuv.loop, p_task->data, p_task->size );
	}
	return 0;
}

int sniff_vms_idle(void *user, void *task)
{
	return 0;
}
