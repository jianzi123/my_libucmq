#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <assert.h>


#if defined __GNUC__
#define likely(x) __builtin_expect ((x), 1)
#define unlikely(x) __builtin_expect ((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif 

typedef int (*TASK_CALLBACK)( void *user, ... );
#ifdef OPEN_CORO
typedef int (*SUPEX_TASK_CALLBACK)( void *user, void *task, int step, ... );
#else
typedef int (*SUPEX_TASK_CALLBACK)( void *user, void *task, ... );
#endif
union virtual_system {
	void *base;
	lua_State* L;			/*if use lua language*/
};

/*
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member)           \
	    ((type*)(((void*)(ptr)) - offsetof(type, member)))
*/

//#ifndef bool
//typedef enum {false, true} bool;
//#endif

#define MIN(x, y)		( ((x)<=(y))?(x):(y) )
#define MAX(x, y)		( ((x)>=(y))?(x):(y) )
#define GET_NEED_COUNT(idx,div)	( (idx) / (div) +  ( ((idx) % (div) >= 1 )? 1 : 0) )
/*************************************************/
#ifdef USE_MUTEX
#define X_NEW_LOCK		pthread_mutex_t
#define X_LOCK_INIT( lock )	pthread_mutex_init( (lock), NULL)
#define X_LOCK( lock )		pthread_mutex_lock( (lock) )
#define X_TRYLOCK( lock )	pthread_mutex_trylock( (lock) )
#define X_UNLOCK( lock )	pthread_mutex_unlock( (lock) )
#else
#define X_NEW_LOCK		pthread_spinlock_t
#define X_LOCK_INIT( lock )	pthread_spin_init( (lock), 0)
#define X_LOCK( lock )		pthread_spin_lock( (lock) )
#define X_TRYLOCK( lock )	pthread_spin_trylock( (lock) )
#define X_UNLOCK( lock )	pthread_spin_unlock( (lock) )
#endif

#define ATOMIC_UNLOCK			0
#define ATOMIC_INLOCK			1

struct safe_once_init {
	int lock;
	int step;
};

#define SAFE_ONCE_INIT_COME( safe ) \
	while ( ! __sync_bool_compare_and_swap(&(safe)->lock, ATOMIC_UNLOCK, ATOMIC_INLOCK) ) {}; \
	do { \
		if ( __sync_fetch_and_add(&(safe)->step, 1) ){ break; }
#define SAFE_ONCE_INIT_OVER( safe ) \
	} while(0); \
	assert ( __sync_bool_compare_and_swap(&(safe)->lock, ATOMIC_INLOCK, ATOMIC_UNLOCK) );
/*************************************************/
#define MAX_FILE_NAME_SIZE	32
#define MAX_FILE_PATH_SIZE	32
#define MAX_API_COUNTS		255
#define MAX_CMD_COUNTS		255
#define MAX_API_NAME_LEN	63
#define MAX_CONNECT		20000
#define MAX_LIMIT_FD		500000//MUST > MAX_CONNECT
#define MAX_DEF_LEN		16384
#define MAX_REQ_SIZE		32768
/*************************************************/
/*==============================================================================================*
 *		log function									*
 *==============================================================================================*/
#define COLOR_NONE            "\x1B[m"
#define COLOR_GRAY            "\x1B[0;30m"
#define COLOR_LIGHT_GRAY      "\x1B[1;30m"
#define COLOR_RED             "\x1B[0;31m"
#define COLOR_LIGHT_RED       "\x1B[1;31m"
#define COLOR_GREEN           "\x1B[0;32m"
#define COLOR_LIGHT_GREEN     "\x1B[1;32m"
#define COLOR_YELLOW          "\x1B[0;33m"
#define COLOR_LIGHT_YELLOW    "\x1B[1;33m"
#define COLOR_BLUE            "\x1B[0;34m"
#define COLOR_LIGHT_BLUE      "\x1B[1;34m"
#define COLOR_PURPLE          "\x1B[0;35m"
#define COLOR_LIGHT_PURPLE    "\x1B[1;35m"
#define COLOR_CYAN            "\x1B[0;36m"
#define COLOR_LIGHT_CYAN      "\x1B[1;36m"
#define COLOR_WHITE           "\x1B[0;37m"
#define COLOR_LIGHT_WHITE     "\x1B[1;37m"

#define LOG_D_VALUE		"[DEBUG]" 
#define LOG_I_VALUE		"[INFO ]"
#define LOG_W_VALUE		"[WARN ]"
#define LOG_F_VALUE		"[FAIL ]"
#define LOG_E_VALUE		"[ERROR]"
#define LOG_S_VALUE		"[SYST ]"

#define LOG_D_LEVEL		0
#define LOG_I_LEVEL		1
#define LOG_W_LEVEL		2
#define LOG_F_LEVEL		3
#define LOG_E_LEVEL		4
#define LOG_S_LEVEL		5

#define LOG_D_COLOR		COLOR_LIGHT_GREEN
#define LOG_I_COLOR		COLOR_LIGHT_GREEN
#define LOG_W_COLOR		COLOR_LIGHT_YELLOW
#define LOG_F_COLOR		COLOR_LIGHT_RED
#define LOG_E_COLOR		COLOR_LIGHT_RED
#define LOG_S_COLOR		COLOR_LIGHT_RED

void init_log( char *path, char *name, int level );

void open_new_log(void);

void dyn_log( int level, const char *value, const char *file, const char *function, int line, const char *fmt, ... );

#define x_perror(msg)		{ fprintf(stderr, COLOR_LIGHT_RED "|-----> "); perror(msg); }

#ifdef OPEN_DEBUG
#define x_printf(lgt, fmt, ...)	fprintf(stdout, LOG_##lgt##_COLOR LOG_##lgt##_VALUE \
					COLOR_LIGHT_CYAN "%16s" COLOR_LIGHT_PURPLE "(%20s)" COLOR_LIGHT_BLUE "|%4d|-->" \
					LOG_##lgt##_COLOR fmt COLOR_NONE, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define x_out_time(x)		{ gettimeofday( ((struct timeval *)x), NULL ); \
					x_printf(S, "time: %ld.%ld s\n", ((struct timeval *)x)->tv_sec, ((struct timeval *)x)->tv_usec); }
#else
#define x_printf(lgt, fmt, args...)	dyn_log( LOG_##lgt##_LEVEL, LOG_##lgt##_VALUE, __FILE__, __FUNCTION__, __LINE__, fmt, ##args )
#define x_out_time(x)		;
#endif


/*==============================================================================================*
 *		time function									*
 *==============================================================================================*/
#define ONE_DAY_TIMESTAMP		(24*60*60)

char *x_strdup(const char *src);

int get_overplus_time(void);

int get_current_time(void);

long long get_system_time(void);

/*==============================================================================================*
 *		fifo function									*
 *==============================================================================================*/
#ifdef OPEN_OPTIMIZE
#define PIPE_MAX_SIZE		((1 << 8)*1024)
#else
#ifndef PIPE_BUFFERS
#define PIPE_BUFFERS            (16)
#endif
#define PIPE_MAX_SIZE		(PIPE_BUF * PIPE_BUFFERS)
#endif
#define PIPE_SIZE_TOP		( PIPE_MAX_SIZE / sizeof(int) )
#define LOOP_SIZE_TOP		(8 * PIPE_SIZE_TOP)

#define FIFO_DATA "-DATA"
#define FIFO_COMD "-COMD"

void fifo_init(int size);

char *get_fifo_msg(void);

char *put_fifo_msg(char *data, int size);

/*==============================================================================================*
 *		msmq function									*
 *==============================================================================================*/
#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define APP_SIGNAL		35
enum {
	MSMQ_LEVEL_PRIOLOW = 1,
	MSMQ_LEVEL_PRIOHIGH,
};
typedef void (*SHELL_CNTL_CB)(const char *data);

#define MSMQ_MAX_DATA_SIZE	512
struct msg_info {//TODO
	char opt;
	char mode;
	int time;
	char data[ MSMQ_MAX_DATA_SIZE ];
};

bool msmq_init(char *name, SHELL_CNTL_CB func);

int msmq_hand(void);

int msmq_call(void);

void msmq_exit(void);

int msmq_send(char *name, char *data, size_t size);

/*==============================================================================================*
 *		pthread function									*
 *==============================================================================================*/
struct safe_init_base {
	int count;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};
struct safe_init_step {
	struct safe_init_base *base;
	int step;
	void *addr;
	void *data;
};
#define SAFE_PTHREAD_INIT_COME( step )	pthread_mutex_lock(&(step)->base->lock)

#define SAFE_PTHREAD_INIT_OVER( step )	(step)->base->count++; \
	pthread_cond_signal(&(step)->base->cond); \
	pthread_mutex_unlock(&(step)->base->lock);

void safe_start_pthread(void *func, int num, void *addr, void *data);
/*==============================================================================================*
 *		free queue									*
 *==============================================================================================*/
struct free_queue_list {
	unsigned int max;
	unsigned int dsz;
	unsigned int isz;
	unsigned int osz;
	void *slots;

	unsigned int head;
	unsigned int tail;
	int w_lock;
	int r_lock;
};

void free_queue_init(struct free_queue_list *list, unsigned int dsz, unsigned int max);

bool free_queue_push(struct free_queue_list *list, void *data);

bool free_queue_pull(struct free_queue_list *list, void *data);
