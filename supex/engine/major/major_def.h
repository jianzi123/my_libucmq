#pragma once
#include "utils.h"

//---> *_cfg.h
#ifdef USE_HTTP_PROTOCOL
enum {
	APPLY_FUNC_ORDER = 0,
	FETCH_FUNC_ORDER,
	MERGE_FUNC_ORDER,
	CUSTOM_FUNC_ORDER,
	LIMIT_FUNC_ORDER,
};
#endif
#ifdef USE_REDIS_PROTOCOL
enum {
	SET_FUNC_ORDER = 0,
	HMGET_FUNC_ORDER,
	HSET_FUNC_ORDER,
	RPUSHX_FUNC_ORDER,
	LPUSHX_FUNC_ORDER,
	HGETALL_FUNC_ORDER,
	LIMIT_FUNC_ORDER,
};
#endif

//---> *_api.h
#ifdef USE_HTTP_PROTOCOL
#include "http.h"
#endif
#ifdef USE_REDIS_PROTOCOL
#include "redis_parser.h"
#endif
/*************************************************/
#define FETCH_MAX_CNT_MSG	"-you have time travel!\r\n"
#define SERVER_BUSY_ALARM_FACTOR		10000
/*************************************************/
#define X_DATA_NO_ALL		2
#define X_DATA_IS_ALL		1

#define X_DONE_OK               0
#define X_IO_ERROR		-1
#define X_DATA_TOO_LARGE	-2
#define X_MALLOC_FAILED		-3
#define X_PARSE_ERROR		-4
#define X_INTERIOR_ERROR	-5
#define X_REQUEST_ERROR		-6
#define X_EXECUTE_ERROR		-7
#define X_REQUEST_QUIT		-8
#define X_KV_TOO_MUCH		-9
#define X_NAME_TOO_LONG		-10
/*************************************************/

#ifdef USE_HTTP_PROTOCOL
struct api_list {
	char type;
	char *name;
	int len;
	TASK_CALLBACK func;
};
#endif
#ifdef USE_REDIS_PROTOCOL
struct cmd_list {
	char type;
	TASK_CALLBACK func;
};
#endif

#include <ev.h>
#include <arpa/inet.h>

#include "list.h"
#include "net_cache.h"
struct data_node {
	/*whenever can't clean when reset data_node*/
	/***base attribute***/
	int sfd;	//用户对应的fd
	

	/*should clean when reset data_node*/
	/***ev***/
	ev_io io_watcher;
	ev_timer timer_watcher;
	/***R***/
	struct net_cache recv;	//用户的接收缓存

	/***W***/
	struct net_cache send;	//用户的发送缓存

	/****S***/
	int control;				/*cmd dispose*/

#ifdef USE_HTTP_PROTOCOL
	struct http_parse_info http_info;
#endif
#ifdef USE_REDIS_PROTOCOL
	struct redis_parse_info redis_info;
#endif
};

struct addr_node {
	enum {
		NO_WORKING = 0,
		IS_WORKING = 1,
	} work_status;				/*io dispose*/
	struct data_node *addr;		//指向data_node,data_node具有收发缓存
	struct queue_item recv_item;
	struct queue_item send_item;
	int port;
	char szAddr[ INET_ADDRSTRLEN ];		/* 255.255.255.255 */
};

void pools_init(void);
struct addr_node *mapping_addr_node(int fd);
struct data_node *get_pool_addr(int fd);
void del_pool_addr(int fd);

void get_cache_data(int sfd, void *buff, int *size);

#define MAX_LISTEN_COUNTS	10240

int socket_init( int port );
