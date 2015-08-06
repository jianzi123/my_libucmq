#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <pthread.h>
#include <termios.h>
#include <semaphore.h>
#include "wrap.h"
#include "cJSON.h"
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


#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define MAX_CONNECT_COUNTS 1000

#define SERV_PORT 8000

#define USE_WHOLE
#ifdef USE_WHOLE
	#define MAX_RECV_SIZE  1024*1024
#else
	#define MAX_RECV_SIZE  1024*8
#endif
#define SPLIT_COPY "SPLIT"
#define WHOLE_COPY "WHOLE"

typedef void *(*CBK_HANDLER_FUN)(void *);
static const char *G_DATA_CP_MODE = NULL;
static const char *G_LUA_FUNC_IN_CMDS = NULL;
static const char *G_LUA_FUNC_OUT_CMDS = NULL;

/*<------------------------------------------------------------------------>*/
#define USE_MUTEX

#ifdef USE_SPINLOCK
pthread_spinlock_t G_SPINLOCK;
#else
pthread_mutex_t G_MUTEX;
#endif
int G_ONLINE_CLIENTS = 0;
/*<------------------------------------------------------------------------>*/
pthread_key_t G_KEY;  
pthread_once_t G_ONCE = PTHREAD_ONCE_INIT;

struct swap_head
{
	int size;
	int msg;
};

struct swap_pool
{
	struct swap_head head;
	char buf[MAX_RECV_SIZE];
};

 
static void destory(void *ptr)  
{
	free(ptr);
}  

static void create(void)
{
	pthread_key_create(&G_KEY, destory);
}

static char *init(void)
{
	pthread_once(&G_ONCE, create);

	char *ptr = NULL;
	ptr = malloc( sizeof( struct swap_pool) );
	memset(ptr, 0, sizeof( struct swap_pool));
	pthread_setspecific(G_KEY, ptr);
	return ptr;
}

static void split_copy_data(void *data, int size)
{
	char *ptr = pthread_getspecific(G_KEY);

	if ( ptr == NULL ) {
		ptr = init();
	}
	memset( ((struct swap_pool *)ptr)->buf, 0, MAX_RECV_SIZE );
	memcpy( ((struct swap_pool *)ptr)->buf, data, size);
	((struct swap_pool *)ptr)->head.size = size;
}
static void whole_copy_data(void *data, int size)
{
	char *ptr = pthread_getspecific(G_KEY);

	if ( ptr == NULL ) {
		ptr = init();
	}
	int uslen = ((struct swap_pool *)ptr)->head.size;
	int lvlen = MAX_RECV_SIZE - uslen;
	int svlen = (lvlen > size ) ? size : lvlen;
	if(svlen > 0){
		memcpy( ((struct swap_pool *)ptr)->buf + uslen, data, svlen);
		((struct swap_pool *)ptr)->head.size += svlen;
	}
}
/*<------------------------------------------------------------------------>*/

int get_data(lua_State *L){
	char *ptr = pthread_getspecific(G_KEY);
	lua_pushlstring( L, (const char* )ptr, ((struct swap_pool *)ptr)->head.size + sizeof(struct swap_head) );
	//lua_pushinteger(L, 5);
	return 1;
}

static void back_func(const char *info)
{
	cJSON *child = NULL;
	cJSON *root = cJSON_Parse(info);
	if (!root) {
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
	}
	else{
		char *ptr = pthread_getspecific(G_KEY);
		child = cJSON_GetObjectItem(root, "size");
		if (child){
			((struct swap_pool *)ptr)->head.size = child->valueint;
		} 
		child = cJSON_GetObjectItem(root, "msg");
		if (child){
			((struct swap_pool *)ptr)->head.msg = child->valueint;
		} 
		cJSON_Delete(root);
	}
}

static int start(lua_State *L)
{

	struct sockaddr_in servaddr;
	int listenfd;
	int ret;
	char busy[] = "server is busy,please try leater!";
	pthread_t id;
	struct sockaddr_in cliaddr;
	socklen_t cliaddr_len;

	int connfd = 0;
	CBK_HANDLER_FUN domain = NULL;

	#ifdef USE_SPINLOCK
		pthread_spin_init(&G_SPINLOCK,0);
	#else
		pthread_mutex_init(&G_MUTEX,NULL);
	#endif

	// --> get pthread working function
	G_DATA_CP_MODE = lua_tostring (L, 1);
	G_LUA_FUNC_IN_CMDS = lua_tostring (L, 2);
	G_LUA_FUNC_OUT_CMDS = lua_tostring (L, 3);
	{
		void *run_lua_string(void *connfd)
		{
			char recv_buf[ MAX_RECV_SIZE ]={0};
			int size;

			//pthred_detach(pthread_self());
			//printf("--------pthread %d-------\n", (int)connfd);
			//char addr[INET_ADDRSTRLEN];
			/*printf("received from %s at PORT %d\n",
			  inet_ntop(AF_INET, &cliaddr.sin_addr, addr, sizeof(addr)),
			  ntohs(cliaddr.sin_port));*/
			init();
			lua_State *L = lua_open();
			luaopen_base(L);
			luaL_openlibs(L);
			lua_register(L, "get_data", get_data);
			while(1){
				memset( recv_buf, 0, MAX_RECV_SIZE );
				size = Read((int)connfd, recv_buf, MAX_RECV_SIZE);//一直阻塞读，除非链接断裂，非阻塞！
				// tcflush((int)connfd,TCIOFLUSH);
				if(size==0){
					// printf("tcp closed!\n");
					break;
				}
				// printf("\n%s\n",recv_buf);
				if ( !strcmp(G_DATA_CP_MODE, SPLIT_COPY) ){
					// printf("split cp\n");
					split_copy_data(recv_buf, size);
				}
				if ( !strcmp(G_DATA_CP_MODE, WHOLE_COPY) ){
					// printf("whole cp\n");
					whole_copy_data(recv_buf, size);
				}

				if (G_LUA_FUNC_IN_CMDS) {
					if(luaL_dostring(L, G_LUA_FUNC_IN_CMDS)){
						const char* errMsg = lua_tostring(L, -1);
						printf("%s\n", errMsg);
						lua_pop(L,1);
					}else{
						if (lua_gettop(L) == 5) {
							// printf("xxxxxxxxxxxxxxxxxxxxxx\n");
							const char *info = lua_tostring(L, -1);
							lua_pop(L, 1);// lua_remove(L, -1);
							back_func(info);
						}
					}
				}
			}
			Close((int)connfd);
			
			
			if (G_LUA_FUNC_OUT_CMDS) {
				if(luaL_dostring(L, G_LUA_FUNC_OUT_CMDS)){
					const char* errMsg = lua_tostring(L, -1);
					printf("%s\n", errMsg);
					lua_pop(L,1);
				}else{
					if (lua_gettop(L) == 5) {
						// printf("-------------------\n");
						const char *info = lua_tostring(L, -1);
						lua_pop(L, 1);// lua_remove(L, -1);
						back_func(info);
					}
				}
			}
			lua_close(L);

			#ifdef USE_SPINLOCK
				pthread_spin_lock(&G_SPINLOCK);
			#else
				pthread_mutex_lock(&G_MUTEX);
			#endif

			G_ONLINE_CLIENTS --;

			#ifdef USE_SPINLOCK
				pthread_spin_unlock(&G_SPINLOCK);
			#else
				pthread_mutex_unlock(&G_MUTEX);
			#endif
			// printf("exit!\n");
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
			Writen(connfd, busy, strlen(busy));
			Close(connfd);
		}
		else{

TODO_PTHREAD_CREATE:
			ret=pthread_create(&id,NULL,(void *)domain, (void *)connfd);
			if(ret != 0)
			{
				printf("pthread_create error! %d \n", ret);
				goto TODO_PTHREAD_CREATE;
			}
			pthread_detach(id);
			#ifdef USE_SPINLOCK
				pthread_spin_lock(&G_SPINLOCK);
			#else
				pthread_mutex_lock(&G_MUTEX);
			#endif

			G_ONLINE_CLIENTS ++;
			
			#ifdef USE_SPINLOCK
				pthread_spin_unlock(&G_SPINLOCK);
			#else
				pthread_mutex_unlock(&G_MUTEX);
			#endif
		}
	}//主线程 return 或exit则进程退出 不加等待子线程完成后进程退出。
	return 0;
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
