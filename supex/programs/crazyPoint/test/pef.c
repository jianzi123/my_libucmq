#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

#define x_out_time(x)		{ gettimeofday( ((struct timeval *)x), NULL ); \
					printf("time: %ld.%ld s\n", ((struct timeval *)x)->tv_sec, ((struct timeval *)x)->tv_usec); }

static struct timeval g_dbg_time;

#if 0
int main(){
	int ret, i;
	ret = sched_yield();
	if(ret == -1){
		printf("调用sched_yield失败!\n");
	}
	return 0;
}
#endif

#if 0
long g_data = 0;

void test(void)
{
	long i = 0;
	for(i=0;i <= 10000000;i++){
		g_data ++;
	}
}

int main(){
	int ret, i;
	ret = nice(-19);
	printf("当前进程的nice value：%d %d\n", ret, errno);
	x_out_time(&g_dbg_time);
	for(i=0;i <= 100;i++){
		test();
	}
	x_out_time(&g_dbg_time);

	ret = nice(100);
	printf("当前进程的nice value：%d %d\n", ret, errno);
	x_out_time(&g_dbg_time);
	for(i=0;i <= 100;i++){
		test();
	}
	x_out_time(&g_dbg_time);
	return 0;
}

#endif
#if 0
long g_data = 0;

void test(void)
{
	long i = 0;
	for(i=0;i <= 10000000;i++){
		g_data ++;
	}
}

int main(){
	int ret, i;
	ret = getpriority(PRIO_PGRP, getpgrp());
	printf("nice value:%d\n", ret);

	x_out_time(&g_dbg_time);
	for(i=0;i <= 100;i++){
		test();
	}
	x_out_time(&g_dbg_time);

	ret = setpriority(PRIO_PGRP, getpgrp(), 10);
	printf("set ok ?:%d\n", ret);
	ret = getpriority(PRIO_PGRP, getpgrp());
	printf("nice value:%d\n", ret);

	x_out_time(&g_dbg_time);
	for(i=0;i <= 100;i++){
		test();
	}
	x_out_time(&g_dbg_time);

	return 0;
}

#endif


#if 0 //good
int main(){
	int ret, i;
	cpu_set_t set;

	CPU_ZERO(&set);
	ret = sched_getaffinity(0, sizeof(cpu_set_t), &set);
	if(ret == -1)
		printf("调用失败!\n");
	for(i = 0; i < 10; i++){
		int cpu = CPU_ISSET(i, &set);
		printf("cpu=%i is %s\n", i, cpu?"set":"unset");
	}

	CPU_ZERO(&set);
	CPU_SET(0, &set);
	CPU_CLR(1, &set);
	ret = sched_setaffinity(0, sizeof(cpu_set_t), &set);
	if(ret == -1)
		printf("调用失败!\n");
	for(i = 0; i < 10; i++){
		int cpu = CPU_ISSET(i, &set);
		printf("cpu=%i is %s\n", i, cpu?"set":"unset");
	}
	return 0;
}
#endif

#if 0
long g_data = 0;

void test(void)
{
	long i = 0;
	for(i=0;i <= 10000000;i++){
		g_data ++;
	}
}


int main()
{
	int i = 0;
	struct sched_param sp = {SCHED_RR};
	if (sched_setscheduler(0, SCHED_FIFO, &sp) < 0){
		perror("fail to sched_setscheduler");
	}

	x_out_time(&g_dbg_time);
	for(i=0;i <= 100;i++){
		test();
	}
	x_out_time(&g_dbg_time);


	sp.__sched_priority = SCHED_FIFO;
	if (sched_setscheduler(0, SCHED_FIFO, &sp) < 0){
		perror("fail to sched_setscheduler");
	}

	x_out_time(&g_dbg_time);
	for(i=0;i <= 100;i++){
		test();
	}
	x_out_time(&g_dbg_time);


	sp.__sched_priority = SCHED_OTHER;
	if (sched_setscheduler(0, SCHED_FIFO, &sp) < 0){
		perror("fail to sched_setscheduler");
	}

	x_out_time(&g_dbg_time);
	for(i=0;i <= 100;i++){
		test();
	}
	x_out_time(&g_dbg_time);

	return 0;
}
#endif


#if 0
int main(){
	int ret, i;
	struct sched_param sp;
	sp.sched_priority = 1;
	ret = sched_setscheduler(0, SCHED_RR, &sp);
	if(ret == -1)
		printf("sched_setscheduler failed.\n");
	if(errno == EPERM)
		printf("Process don't the ability.\n");

	ret = sched_getscheduler(0);
	switch(ret){
		case SCHED_OTHER:
			printf("Policy is normal.\n");
			break;
		case SCHED_RR:
			printf("Policy is round-robin.\n");
			break;
		case SCHED_FIFO:
			printf("Policy is first-in, first-out.\n");
			break;
		case -1:
			printf("sched_getscheduler failed.\n");
			break;
		default:
			printf("Unknow policy\n");
	}
	return 0;
}
#endif


int main()
{
	while(1){
		sleep(1);
	}
	return 1;
}
/*renice -19 -p pid*/
