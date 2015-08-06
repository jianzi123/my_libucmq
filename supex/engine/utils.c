#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <mqueue.h>

#include "utils.h"

char *x_strdup(const char *src)
{ 
	if (src == NULL)
		return NULL;

	int len = strlen(src);
	char *out = calloc(len + 1, sizeof(char));
	strcpy(out, src);
	return out; 
}

/*********************************TIME***********************************************/

/*
 * 名 称:ger_overplus_time
 * 功 能:
 * 参 数:
 * 返回值:
 * 修改:
 */
int get_overplus_time(void)
{
	static int g_mon = -1;

	time_t t_now = time(NULL);
	struct tm *p_tm = localtime(&t_now);
	struct tm tm = {0};
	if (g_mon != p_tm->tm_mon) {
		g_mon = p_tm->tm_mon;
		x_printf(D, "current month:             %d\n", g_mon);
		return 0;
	}
	tm.tm_mon = (p_tm->tm_mon + 1) % 12;
	tm.tm_year = (p_tm->tm_mon == 11) ? (p_tm->tm_year + 1):( p_tm->tm_year);
	time_t t_end = timelocal(&tm);
	int space = t_end - t_now;
	return (space > 0)?space:1;
}

int get_current_time(void)
{
	struct timeval tv = {0};
	gettimeofday(&tv, NULL);
	return (tv.tv_sec + 8*60*60) % (24*60*60);
	//return ((tv.tv_sec + 8*60*60) % (ONE_DAY_TIMESTAMP) + 12660)% (ONE_DAY_TIMESTAMP);/*just for test 24:00 come!*/
}

long long get_system_time(void)
{
    struct timeb t;
    ftime(&t);
    return 1000 * t.time + t.millitm;
}
#if 0
#include <time.h>
static inline unsigned long long _get_time_cycle( void )
{
	__asm ("RDTSC");
}

static int get_array_random_index(int max)//not effect
{
	long long count = _get_time_cycle();
	return ( count % max );
}
#endif

/*********************************LOGS***********************************************/
#define MIN_LOGFD_RECORD_PEAK	4//should >= 4 or max pthread counts call 'open_new_log'
struct log_file {
	int level;
	int nowfd;
	unsigned int index;
	int logfd[ MIN_LOGFD_RECORD_PEAK ];
	char path[ MAX_FILE_PATH_SIZE ];
	char name[ MAX_FILE_NAME_SIZE ];
};

static struct log_file g_log_file = {};
static struct safe_once_init g_init_log_mark = {};

void init_log( char *path, char *name, int level )
{
	SAFE_ONCE_INIT_COME( &g_init_log_mark );

	memset(&g_log_file, 0, sizeof(struct log_file));
	g_log_file.level = level;
	strncat(g_log_file.path, path, MAX_FILE_PATH_SIZE - 1);
	strncat(g_log_file.name, name, MAX_FILE_NAME_SIZE - 1);

	SAFE_ONCE_INIT_OVER( &g_init_log_mark );
}

/*
 * 函 数:open_new_log
 * 功 能:打开日志
 */
void open_new_log(void)
{
	int newfd, oldfd = 0;
	time_t t_now = time(NULL);
	struct tm *p_tm = localtime(&t_now);
	char name[64] = {0};
	snprintf(name, (sizeof(name) - 1), "%s%s_%04d%02d.log", g_log_file.path, g_log_file.name, 1900 + p_tm->tm_year, 1 + p_tm->tm_mon);
	newfd = open(name, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR|S_IWUSR);

	int offset = __sync_fetch_and_add(&g_log_file.index, 1) % MIN_LOGFD_RECORD_PEAK;
	do {
		oldfd = g_log_file.logfd[ offset ];
		if (oldfd > 0){
			close( oldfd );
		}
		__sync_lock_test_and_set( &g_log_file.nowfd, newfd );
	}while( !__sync_bool_compare_and_swap(&g_log_file.logfd[ offset ], oldfd, newfd) );
}


void dyn_log( int level, const char *value, const char *file, const char *function, int line, const char *fmt, ... )
{
	if (level < g_log_file.level)
		return; /* too verbose */
	/* get current time and log level */
	time_t now = time(NULL);
	char time_buf[32] = {0};
	strftime(time_buf, sizeof(time_buf), "%y%m%d_%H%M%S", localtime(&now));
	dprintf(g_log_file.nowfd, "%s %s%16s(%20s)|%4d|-->", time_buf, value, file, function, line);
	/*write mesage*/
	va_list ap;
	va_start(ap, fmt);
	vdprintf(g_log_file.nowfd, fmt, ap);
	va_end(ap);
	/* write to log and flush to disk. */
#if 0
	fsync(g_log_file.nowfd);
	//fdatasync(g_log_file.nowfd);
#endif
}


/*********************************FIFO***********************************************/
static char *g_fifo_data = NULL;
static char *g_fifo_comd = NULL;

void fifo_init(int size)
{
	g_fifo_data = (char *)malloc( size );
	g_fifo_comd = (char *)malloc( size );
	
	unlink( FIFO_DATA );
	unlink( FIFO_COMD );
	if(mkfifo(FIFO_DATA, S_IFIFO|0666) || mkfifo(FIFO_COMD, S_IFIFO|0666))
	{
		x_printf(E, "cannot create fifo \n");
		unlink( FIFO_DATA );
		unlink( FIFO_COMD );
		exit(0);
	}
}

char *get_fifo_msg(void)
{
	int fd,n = 0;
	fd = open(FIFO_DATA, O_RDONLY, 0);
	if(fd < 0){
		x_printf(E, "open for read error\n");
		unlink(FIFO_DATA);
		exit(0);
	}
	n = read(fd, g_fifo_data, getpagesize());
	x_printf(D, "read %d : %s \n", n, g_fifo_data);
	close(fd);


	fd = open(FIFO_COMD, O_WRONLY, 0);
	if(fd < 0){
		if(errno == ENXIO){
			x_printf(E, "open error; no reading process \n");
		}
		else if(errno == EEXIST){
			x_printf(E, "open error: file not exist \n");
		}
		perror("error \n");
	}
	n = write(fd,"ok\n", 3);
	x_printf(D, "write %d : ok\\n \n", n);
	close(fd);

	return g_fifo_data;
}
char *put_fifo_msg(char *data, int size)
{
	int fd,n = 0;
	fd = open(FIFO_DATA, O_WRONLY, 0);
	if(fd < 0){
		if(errno == ENXIO){
			x_printf(E, "open error; no reading process \n");
		}
		else if(errno == EEXIST){
			x_printf(E, "open error: file not exist \n");
		}
		perror("error \n");
	}
	n = write(fd, data, size);
	x_printf(D, "write %d : %s \n", n, data);
	close(fd);

	fd = open(FIFO_COMD, O_RDONLY, 0);
	if(fd < 0){
		x_printf(E, "open for read error\n");
		unlink(FIFO_COMD);
		exit(0);
	}
	n= read(fd, g_fifo_comd, getpagesize());
	x_printf(D, "read %d : %s \n", n, g_fifo_comd);
	//TODO checkok
	close(fd);

	return g_fifo_comd;
}

/*********************************MSMQ***********************************************/
typedef struct msmq_info {
	int pfds[2];
	mqd_t mqd;
	struct sigevent sgev;
	int size;
	char *buf;
	SHELL_CNTL_CB fcb;
	char name[ MAX_FILE_NAME_SIZE ];
} MSMQ_INFO;

static MSMQ_INFO g_msmq_info = {};
static struct safe_once_init g_msmq_init_mark = {};


static void app_signal_callback(int signo)
{
	x_printf(D, "----------------------\n");
	write(g_msmq_info.pfds[1], "", 1);
	return;
}

bool msmq_init(char *name, SHELL_CNTL_CB func)
{
	bool ok = false;
	SAFE_ONCE_INIT_COME( &g_msmq_init_mark );

	strncat(g_msmq_info.name, name, MAX_FILE_NAME_SIZE - 1);
	g_msmq_info.fcb = func;
	/*create pipe*/
	if( pipe2(g_msmq_info.pfds, (O_NONBLOCK | O_CLOEXEC)) < 0 ){
		perror("pipe2");
		exit(EXIT_FAILURE);
	}

	/*register signal*/
	signal(APP_SIGNAL, app_signal_callback);
	g_msmq_info.sgev.sigev_notify = SIGEV_SIGNAL;
	g_msmq_info.sgev.sigev_signo = APP_SIGNAL;


	/*init msmq*/
	g_msmq_info.mqd = mq_unlink( g_msmq_info.name );
	g_msmq_info.mqd = mq_open( g_msmq_info.name, O_CREAT|O_RDONLY|O_NONBLOCK, FILE_MODE, NULL );
	if (g_msmq_info.mqd == (mqd_t)-1) {
		perror("mq_open");
		exit(EXIT_FAILURE);
	}
	/*get max size*/
	struct mq_attr attr;
	mq_getattr(g_msmq_info.mqd, &attr);
	g_msmq_info.size = attr.mq_msgsize;
	g_msmq_info.buf = (char *)malloc(g_msmq_info.size);
	memset(g_msmq_info.buf, 0, g_msmq_info.size);

	/*register msmq*/
	if (mq_notify(g_msmq_info.mqd, &g_msmq_info.sgev) == -1 ){
		perror("mq_notify");
		exit(EXIT_FAILURE);
	}

	ok = true;
	SAFE_ONCE_INIT_OVER( &g_msmq_init_mark );
	return ok;
}

int msmq_hand(void)
{
	return g_msmq_info.pfds[0];
}

int msmq_call(void)
{
	char mark;
	ssize_t size;
	unsigned int prio;
	int num = 0;

	x_printf(D, ">>>>>>>>>>>>>>>>>>>>>>\n");


	while(1){
		int nfds = read(g_msmq_info.pfds[0], &mark, 1);
		if (nfds == 1){
			x_printf(D, "++++++++++++++++++++++\n");
			if (mq_notify(g_msmq_info.mqd, &g_msmq_info.sgev) == -1 ){
				perror("mq_notify");
				exit(EXIT_FAILURE);
			}
			while ((size = mq_receive(g_msmq_info.mqd, g_msmq_info.buf, g_msmq_info.size, &prio)) >= 0) {
				num++;
				x_printf(D, "read %ld bytes\n", (long)size);

				g_msmq_info.fcb( g_msmq_info.buf );

				memset(g_msmq_info.buf, 0, g_msmq_info.size);
			}
			if (errno != EAGAIN){
				x_printf(E, "error hapend!!!\n");
			}
		}else{
			break;
		}
	}
	x_printf(D, "<<<<<<<<<<<<<<<<<<<<<<\n");

	return num;
}

int msmq_send(char *name, char *data, size_t size)
{
	mqd_t mqd = mq_open(name, O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL);
	if (mqd == (mqd_t)-1) {
		x_printf(E, "mq_open %s failed\n", name);
		return -1;
	}
	mq_send(mqd, data, size, MSMQ_LEVEL_PRIOLOW);
	mq_close(mqd);
	return 0;
}

void msmq_exit(void)
{
	mq_close(g_msmq_info.mqd);
	free(g_msmq_info.buf);
}

/*********************************THREAD***********************************************/
/*
 *函 数:safe_start_pthread
 *功 能:
 *参 数:func线程入口函数，num开启线程的个数,addr指向一个数组，数组中的每个成员都是一个指针，指向PARTER_PTHREAD,data指向绑定数据，所有的parter线程都指向同一个bind绑定数据
 *返回值:
 */
void safe_start_pthread(void *func, int num, void *addr, void *data)
{
	pthread_attr_t  attr;
	pthread_t       thread;
	
	struct safe_init_base base = {};

	pthread_mutex_init(&base.lock, NULL);
	pthread_cond_init(&base.cond, NULL);
	
	struct safe_init_step slot[ num ];
	//启动num个线程
	int i = 0;
	for (i = 0; i < num; i++) {
		slot[i].base = &base;
		slot[i].step = i;
		slot[i].addr = addr;
		slot[i].data = data;

		pthread_attr_init(&attr);
		/* create thread */
		if (0 != pthread_create(&thread, &attr, func, (void *)&slot[i])) {
			x_perror("Can't create thread");
			exit(EXIT_FAILURE);
		}
	}
	/* Wait for all the threads to set themselves up before returning. */
	pthread_mutex_lock(&base.lock);
	while ( base.count < num ) {
		pthread_cond_wait(&base.cond, &base.lock);
	}
	pthread_mutex_unlock(&base.lock);
	sleep(1);/*just to delay*/
	return;
}




/*********************************QUEUE***********************************************/
void free_queue_init(struct free_queue_list *list, unsigned int dsz, unsigned int max)
{
	memset(list, 0, sizeof(struct free_queue_list));
	list->max = max;
	list->dsz = dsz;
	list->isz = 0;
	list->osz = 0;
	list->slots = calloc( max, dsz );
	assert(list->slots);

	list->head = 0;
	list->tail = 0;
	list->w_lock = ATOMIC_UNLOCK;
	list->r_lock = ATOMIC_UNLOCK;
	return;
}
/*free lock is just read and write don't need lock*/
bool free_queue_push(struct free_queue_list *list, void *data)
{
	bool ok = false;
	assert( list && data );

	while ( ! __sync_bool_compare_and_swap(&list->w_lock, ATOMIC_UNLOCK, ATOMIC_INLOCK) ) {};

	unsigned int next = (list->tail + 1) % list->max;
	if ( list->head != next ){
		memcpy( &((char *)list->slots)[ list->tail * list->dsz ], (char *)data, list->dsz );
		list->tail = next;
		list->isz ++;
		ok = true;
	}
	
	assert ( __sync_bool_compare_and_swap(&list->w_lock, ATOMIC_INLOCK, ATOMIC_UNLOCK) );
	return ok;
}

bool free_queue_pull(struct free_queue_list *list, void *data)
{
	bool ok = false;
	assert( list && data );

	while ( ! __sync_bool_compare_and_swap(&list->r_lock, ATOMIC_UNLOCK, ATOMIC_INLOCK) ) {};

	if(list->head != list->tail) {
		memcpy( data, &((char *)list->slots)[ list->head * list->dsz ], list->dsz );
		list->head = (list->head + 1) % list->max;
		list->osz ++;
		ok = true;
	}

	assert ( __sync_bool_compare_and_swap(&list->r_lock, ATOMIC_INLOCK, ATOMIC_UNLOCK) );
	return ok;
}
