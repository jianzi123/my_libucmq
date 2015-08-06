#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <pthread.h>
#include <termios.h>
#include <semaphore.h>
#include "wrap.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/wait.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


#define MAX_CONNECT_COUNTS 1000

#define SERV_PORT 8000

#define MAX_RECV_SIZE  1024*8

#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define SEMNAME "tcpsrv"
typedef void *(*CBK_HANDLER_FUN)(void *);
static const char *lua_func_cmds = NULL;
int G_ONLINE_CLIENTS = 0;
sem_t  *G_MUTEX = NULL;
/*<------------------------------------------------------------------------>*/
sem_t *_sem_create(const char *name)
{
	sem_t  *mutex;
	if((mutex = sem_open(name, O_CREAT|O_EXCL,FILE_MODE, 1)) == SEM_FAILED)
	{
		printf("sem_open error");
		exit(-1);
	}
	return mutex;
}
sem_t *_sem_open(const char *name)
{
	sem_t  *mutex;
	if((mutex = sem_open(name, 0)) == SEM_FAILED)
	{
		printf("sem_open error");
		exit(-1);
	}
	return mutex;
}

void _sem_lock(sem_t  *mutex)
{
	sem_wait(mutex);
}

void _sem_unlock(sem_t  *mutex)
{
	sem_post(mutex);
}

void _sem_close(sem_t  *mutex)
{
	sem_close(mutex);
}

void _sem_delete(const char *name)
{
	sem_unlink(name);
}

/*<------------------------------------------------------------------------>*/
pthread_key_t key;  
pthread_once_t once = PTHREAD_ONCE_INIT;  
 
static void destructor(void *ptr)  
{  
	free(ptr);  
}  
 
static void init_once(void)  
{  
	pthread_key_create(&key, destructor);  
}  
 
static void *get_buf(void)  
{
	char *ptr = pthread_getspecific(key);
	return ptr;
}  
 
static void copy_data(void *data)  
{
	char *ptr = NULL;
	pthread_once(&once, init_once);  
	
	if ((ptr = pthread_getspecific(key)) == NULL) {  
		ptr = malloc(MAX_RECV_SIZE);  
		pthread_setspecific(key, ptr);  
	}
	memset(ptr, 0, MAX_RECV_SIZE);
	memcpy(ptr, data, MAX_RECV_SIZE);
}  
/*<------------------------------------------------------------------------>*/
struct sockaddr_in cliaddr;
socklen_t cliaddr_len;//FIXME


int get_data(lua_State *L){
	char *ptr = get_buf();
	lua_pushlstring(L, (const char* )ptr, strlen(ptr));
	//lua_pushinteger(L, 5);
	return 1;
}


static int start(lua_State *L)
{

	struct sockaddr_in servaddr;
	int listenfd;
	int ret , j;
	char *buff;
	pthread_t id;

	int connfd = 0;
	CBK_HANDLER_FUN domain = NULL;

	//delete semaphore
	_sem_delete(SEMNAME);
	//create semaphore
	G_MUTEX = _sem_create(SEMNAME);
	//close semaphore
	_sem_close(G_MUTEX);

	// --> get pthread working function
	lua_func_cmds = lua_tostring (L, 1);
	{
		void *run_lua_string(void *connfd)
		{
			char recv_buf[ MAX_RECV_SIZE ]={0};
			char str[INET_ADDRSTRLEN];
			int n ;
			while(1){
				n = Read((int)connfd, recv_buf, MAX_RECV_SIZE);//一直阻塞读，除非链接断裂，非阻塞！
				//	tcflush((int)connfd,TCIOFLUSH);
				if(n==0){
					printf("用户中断！\n");
					break;
				}
				copy_data(recv_buf);
				/*printf("received from %s at PORT %d\n",
						inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
						ntohs(cliaddr.sin_port));*/
				//printf("\n%s\n",recv_buf);


				//TODO
				printf("--------pthread %d-------\n", (int)connfd);
				lua_State *L = lua_open();
				luaopen_base(L);
				luaL_openlibs(L);
				lua_register(L, "get_data", get_data);
				if(luaL_dostring(L, lua_func_cmds)){
					const char* errMsg = lua_tostring(L, -1);
					printf("%s\n", errMsg);
				};
				lua_close(L);
			}
			printf("exit!\n");
			Close((int)connfd);
			G_MUTEX = _sem_open(SEMNAME);
			_sem_lock(G_MUTEX);
			G_ONLINE_CLIENTS --;
			_sem_unlock(G_MUTEX);
			_sem_close(G_MUTEX);
			pthread_exit(0);//子线程exit 进程退出 return则不会。 
		}
		domain = run_lua_string;
	}

	// --> start tcp service
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//0.0.0.0
	servaddr.sin_port = htons(SERV_PORT);

	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));//绑定8000端口

	Listen(listenfd, 20);//监听的是listenfd（0.0.0.0:8000）

	printf("Accepting connections ...\n");
	while (1) {

		cliaddr_len = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);//TCP连接指的是 connfd（客户端ip:8000）
		if( G_ONLINE_CLIENTS >= MAX_CONNECT_COUNTS )
		{
			Writen(connfd, buff= "服务器忙，请稍候再试！", strlen(buff));
			Close(connfd);
		}
		else{

			ret=pthread_create(&id,NULL,(void *)domain, (void *)connfd);
			if(ret != 0)
			{
				printf("pthread_create error!\n");
				return -1;
			}
			G_MUTEX = _sem_open(SEMNAME);
			_sem_lock(G_MUTEX);
			G_ONLINE_CLIENTS ++;
			_sem_unlock(G_MUTEX);
			_sem_close(G_MUTEX);
		}
	}//主线程 return 或exit则进程退出 不加等待子线程完成后进程退出。
}

static const struct luaL_Reg lib[] =
{
	{"start", start},
	{NULL, NULL}
};

int luaopen_tcpsrv(lua_State *L) {
	luaL_register(L, "tcpsrv", lib);
	return 1;
}
