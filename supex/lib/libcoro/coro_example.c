/*
* 用ucontext实现简单的用户空间协作多线程
* 
* 我在很早以前的一篇文章中介绍过如何用setjmp/longjmp实现简单的协作多任务，并且也提到用非透明的数据结构jmp_buf的内部成员来实现此项操作实属违规。前些天，偶然在一个讨论组上看到有人提到用ucontext实现用户级线程以规避内核级线程创建及调度的开销和纯异步编程中状态机的复杂性，突然觉得眼前一亮。如果联系到目前Linux内核正在开发的syslet，用户空间程序通过协同的方式确实能够达到高效编程和高效执行的目的。比如读操作:
* 
* 1.设置要读的文件描述符fd为非阻塞方式
* 2.调用读操作，如果成功，函数返回。否则，执行3
* 3.用select系列函数监听fd的可读事件，然后设置当前线程状态为阻塞，调用schedule让出CPU
* 4.fd可读时将线程状态变成可运行，以供以后调度
* 
* 所以，今天又重新实现了用户空间的协作多线程示例:
*/

#include <stdlib.h>
#include <stdio.h>
#include "coro.h"

struct args {
	int n;
};

static void *foo(struct schedule * S, void *ud) {
	struct args * arg = ud;
	int start = arg->n;
	int i;
	printf("--->>%d\n", ((struct args *)ud)->n);
	coroutine_switch(S);
	printf("<<---%d\n", ((struct args *)ud)->n);
	return NULL;
}

struct coroutine *c1;
struct coroutine *c2;
struct coroutine *c3;
struct coroutine *c4;

void *get_list(struct schedule *S, void *data){
	static id = 0;
	if (id != 0)
		return NULL;
	id ++;
	return (void *)c1;
}
int main(void)
{
	//int i = 0;
	struct args arg1 = { 0 };
	struct args arg2 = { 100 };
	struct args arg3 = { 200 };
	struct args arg4 = { 300 };


	struct schedule *S = coroutine_open( NULL, get_list, NULL );
	//while(i++<=1000000){
		c1 = coroutine_create(S, foo, &arg1);
		c2 = coroutine_create(S, foo, &arg2);
		c3 = coroutine_create(S, foo, &arg3);
		c4 = coroutine_create(S, foo, &arg4);
		c1->next = c2;
		c2->next = c3;
		c3->next = c4;
		printf("main start\n");
		coroutine_loop(S);
		printf("main end\n");
	//}
	coroutine_close(S);
	return 0;
}

