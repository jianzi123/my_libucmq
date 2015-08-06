#pragma once

#include "coro.h"

#define TCP_ERR_CONNECT		-1
#define TCP_ERR_SEND		-2
#define TCP_ERR_SOCKOPT		-3
#define TCP_ERR_MEMORY		-4
#define TCP_ERR_TIMEOUT		-5

#define DEFAULT_RECV_SIZE 20480

#define MAX_CORO_SWITCH_TIME_OUT	3500

int x_connect(const char *host, int port);

int sync_tcp_ask(const char *host, short port, const char *data, size_t size, char **back, size_t time);

int async_tcp_ask(const char *host, short port, const char *data, size_t size, char **back, size_t time, struct schedule *S);
