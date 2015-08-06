/*
 * auth: baoxue
 * date: Sat Aug  3 10:20:26 CST 2013
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>

#include "tcp_api.h"

static int _connect_server(const char *host, int client_port) {
	int sockfd = 0;
	struct timeval timeout;
	timeout.tv_sec = 0;//0秒
	timeout.tv_usec = 500000;//0.5秒
	struct linger quick_linger;
	quick_linger.l_onoff = 1;
	quick_linger.l_linger = 0;
	

	unsigned long inaddr = inet_addr(host);
	if (inaddr != INADDR_NONE) {
		/* normal IP */
		struct sockaddr_in ad;
		memset(&ad, 0, sizeof(ad));
		ad.sin_family = AF_INET;

		memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
		ad.sin_port = htons(client_port);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			return -1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, (socklen_t)sizeof(struct timeval)) == -1){
			perror("setsockopt()");
			close(sockfd);
			return -1;
		}
		if (connect(sockfd, (struct sockaddr *)&ad, sizeof(ad)) == 0){
			return sockfd;
		}
		setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
		close(sockfd);
		return -1;
	} else {
		char port[6]; /* 65535 */
		struct addrinfo hints, *res, *ressave;

		snprintf(port, 6, "%d", client_port);
		memset(&hints, 0, sizeof(hints));

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		/* not IP, host name */
		if ( getaddrinfo(host, port, &hints, &res) != 0 ) {
			return -1;
		}

		ressave = res;/*must not NULL*/

		do {
			sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
			if (sockfd < 0)
				continue; /* ignore this one */
			if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, (socklen_t)sizeof(struct timeval)) == -1){
				perror("setsockopt()");
			}else{
				if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0){
					freeaddrinfo(ressave);
					return sockfd;
				}
			}
			setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
			close(sockfd);
		} while ((res = res->ai_next) != NULL);

		freeaddrinfo(ressave);
		return -1;
	}
}

int http_get_content_length(char *buf) {
	char *p = strstr( buf , "Content-length:" );
	if( !p )
		p = strstr( buf , "content-Length:" );
	if( !p )
		p = strstr( buf , "Content-Length:" );
	if( !p )
		p = strstr( buf , "content-length:" );
	if( !p ){
		return 0;
	}
	char *rn = strstr(p , "\r\n");
	if( !rn ){
		return 0;
	}
	char *d = strstr(p , " ");
	if( !d ){
		d = strstr(p , ":");
	}
	if( !d ){
		return 0;
	}
	return atol(d);
}
int http_get_head_length(char *buf) {
    char *p = strstr( buf , "\r\n\r\n" );
    if( !p ){
	    return 0;
    }
    return (p - buf + 4);
}

int sync_tcp_ask(const char *host, short port, const char *data, size_t size, char **back, size_t time)
{
	struct linger quick_linger;
	quick_linger.l_onoff = 1;
	quick_linger.l_linger = 0;
	/*
	 * struct linger delay_linger;
	 * delay_linger.l_onoff = 1;
	 * delay_linger.l_linger = 1;
	 */

	int sock = _connect_server(host, port);
	if (sock == -1) {
		printf("connect server(%s:%d) fail", host, port);
		return TCP_ERR_CONNECT;
	}

	size_t io_size = 0;
	int bytes = 0;

	printf("out data:%s\n", data);
	while (size != io_size){
		bytes = send(sock, data + io_size,
				size - io_size,
				0);
		printf("-------> send = %d\n",  bytes);
		if (bytes < 0){
			printf("discard all\n");
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
			close(sock);
			return TCP_ERR_SEND;
		}
		io_size += bytes;
	}
	if (!back){
		close(sock);
		return 0;
	}
	if (time > 0){
		struct timeval timeout;
		timeout.tv_sec = time / 1000000;
		timeout.tv_usec = time % 1000000;
		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, (socklen_t)sizeof(struct timeval)) == -1){
			close(sock);
			printf("can't set recv timeout!\n");
			return TCP_ERR_SOCKOPT;
		}
	}

	char temp[ DEFAULT_RECV_SIZE ] = {0};
	size_t max_size = DEFAULT_RECV_SIZE;

	char *p_now = temp;
	char *p_new = NULL;
	bytes = 0;
	io_size = 0;
	while (1) {
		bytes = recv(sock, p_now + io_size, max_size - io_size, 0);
		if (bytes > 0) {
			io_size += bytes;
			printf("in data:%d\n", bytes);
		}else if(bytes == 0){/* socket has closed when read after */
			printf("remote socket closed!socket fd: %d\n", sock);
			break;
		}else{
			if(errno == EAGAIN ||errno == EWOULDBLOCK){
				continue;
			}else{/* socket is going to close when reading */
				printf("ID is %d\n", errno);
				printf("EINTR is %d\n", EINTR);
				printf("ret :%d ,close socket fd : %d\n", bytes, sock);
				break;
			}
		}

		/* reach default receive buffer size */
		if (io_size + 32 >= max_size) {
			max_size *= 2;
			p_new = calloc(max_size, 1);
			if (p_new == NULL) {
				setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
				close(sock);
				if(p_now != temp){
					free(p_now);
				}
				printf("no more memory!\n");
				return TCP_ERR_MEMORY;
			}
			memcpy(p_new, p_now, io_size);
			if(p_now != temp){
				free(p_now);
			}
			p_now = p_new;
		}
	}
	if(p_now == temp){
		p_now = calloc(io_size, 1);
		if (p_now == NULL) {
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
			close(sock);
			printf("no more memory!\n");
			return TCP_ERR_MEMORY;
		}
		memcpy(p_now, temp, io_size);
	}
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&quick_linger, sizeof(quick_linger));
	close(sock);
	*back = p_now;
	printf("%s\n", p_now);
	return io_size;
}


void main(void)
{
	char *data = "GET /11 HTTP/1.0\r\nUser-Agent: curl/7.32.0\r\nHost: 127.0.0.1:8088\r\nAccept: */*\r\n\r\n";
	char *ret = NULL;
	sync_tcp_ask("127.0.0.1", 8088, data, strlen(data), &ret, 10000000);
}
