#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>


#include "major_def.h"

static struct addr_node g_addr_slot[ MAX_LIMIT_FD ] = {};
static struct data_node *g_data_pool = NULL;	//客户端data_node
static struct safe_once_init g_pools_init_mark = {};

/*
 *函 数:pools_init
 *功 能:
 */
void pools_init(void)
{
	assert( MAX_LIMIT_FD >= MAX_CONNECT );
	SAFE_ONCE_INIT_COME( &g_pools_init_mark );

	int i;
	//最多创建MAX_CONNECT个客户端信息
	g_data_pool = calloc( MAX_CONNECT, sizeof(struct data_node) );
	for(i=0; i<MAX_CONNECT; i++){
		g_data_pool[i].sfd = i;
#ifdef USE_HTTP_PROTOCOL
		g_data_pool[i].http_info.hp.data = &g_data_pool[i];//TODO
#endif
		cache_init( &g_data_pool[i].recv );
		cache_init( &g_data_pool[i].send );

		g_addr_slot[ i ].addr = &g_data_pool[i];
	}
	for(i=0; i<MAX_LIMIT_FD; i++){
		g_addr_slot[ i ].recv_item.data = i;
		g_addr_slot[ i ].send_item.data = i;
		g_addr_slot[ i ].work_status = NO_WORKING;
	}
	SAFE_ONCE_INIT_OVER( &g_pools_init_mark );
}

/*
 *函 数:mapping_addr_node
 *功 能:从g_addr_slot中取出槽位
 */
struct addr_node *mapping_addr_node(int fd)
{
	return &g_addr_slot[ fd ];
}

/*
 *函 数:get_pool_addr
 *功 能:为槽位创建data_node，并初始化之后返回data_node
 */
struct data_node *get_pool_addr(int fd)
{
	assert( MAX_LIMIT_FD > fd );
	if ( (fd >= MAX_CONNECT) &&
			(NULL == g_addr_slot[ fd ].addr) ){
		g_addr_slot[ fd ].addr = calloc( 1, sizeof(struct data_node) );
		struct data_node *p_node = g_addr_slot[ fd ].addr;
		assert(p_node);
		p_node->sfd = fd;
#ifdef USE_HTTP_PROTOCOL
		p_node->http_info.hp.data = p_node;//TODO
#endif
		cache_init( &p_node->recv );
		cache_init( &p_node->send );
	}
	return g_addr_slot[ fd ].addr;
}


void del_pool_addr(int fd)
{
	assert( MAX_LIMIT_FD > fd );
	if ( (fd >= MAX_CONNECT) &&
			(NULL != g_addr_slot[ fd ].addr) ){
		free( g_addr_slot[ fd ].addr );
		g_addr_slot[ fd ].addr = NULL;
	}
}


void get_cache_data(int sfd, void *buff, int *size)
{
	struct data_node *p_node = get_pool_addr( sfd );
	struct net_cache *p_cache = &p_node->recv;
	memcpy(buff, p_cache->buf_addr, p_cache->get_size);
	*size = p_cache->get_size;
	return;
}


int socket_init( int port )
{
	struct sockaddr_in my_addr;
	int listenfd;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		x_perror("socket");
		exit(EXIT_FAILURE);
	} 
	else{
		x_printf(S, "SOCKET CREATE SUCCESS!\n");
	}

	/* set nonblock */
	fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL) | O_NONBLOCK);
	/* set reuseaddr */
	int so_reuseaddr = 1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(so_reuseaddr));
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = PF_INET;
	my_addr.sin_port = htons( port );
	my_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");

	if (bind(listenfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))== -1) {
		x_perror("bind");
		exit(EXIT_FAILURE);
	} 
	else{
		x_printf(S, "IP BIND SUCCESS!\n");
	}

	if (listen(listenfd, MAX_LISTEN_COUNTS) == -1) {
		x_perror("listen");
		exit(EXIT_FAILURE);
	} 
	else{
		x_printf(S, "LISTEN SUCCESS,PORT:%d\n", port);
	}
	return listenfd;
}
