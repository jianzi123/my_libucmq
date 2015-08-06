#pragma once

#include "net_cache.h"
#include "redis_parser.h"
#include "http.h"

#define ASYNC_OK	0
#define ASYNC_ERR	-1


struct async_ctx;

typedef void (*ASYNC_CALL_BACK)(struct async_ctx *ac, void *reply, void *data);

#ifdef OPEN_PUB_SUB
struct command_node {
	ASYNC_CALL_BACK fcb;
	void *privdata;

	struct command_node *next; /* simple singly linked list */
};
struct command_list {
	struct net_cache cache;
	struct command_node *head;
	struct command_node *tail;
};
#else
struct command_node {
	struct net_cache cache;

	ASYNC_CALL_BACK fcb;
	void *privdata;

	struct command_node *next; /* simple singly linked list */
};
struct command_list {
	struct command_node base;
	struct command_node *head;
	struct command_node *tail;
	struct command_node *work;
};
#endif

enum proto_type {
        PROTO_TYPE_HTTP = 0,
        PROTO_TYPE_REDIS
};


/* Connection callback prototypes */
typedef void (LINK_CALL_BACK)(const struct async_ctx*, void *data);
typedef void (RECY_CALL_BACK)(const struct async_ctx*, void *data);

/** async protocol callback prototype */
typedef int (PROTO_CALL_BACK)(struct async_ctx*);


/* Context for an async connection to Redis */
struct async_ctx {
	int sfd;
	void *with_data;
	/* Event library data and hooks */
	struct {
		void *data;

		/* Hooks that are called when the library expects to start
		 * reading/writing. These functions should be idempotent. */
		void (*add_recv)(void *privdata);
		void (*del_recv)(void *privdata);
		void (*add_send)(void *privdata);
		void (*del_send)(void *privdata);
		void (*cleanup)(void *privdata);
	} ev;

	void *argv_data;
        PROTO_CALL_BACK *proto_handler;
	LINK_CALL_BACK *disconnect;
	LINK_CALL_BACK *getconnect;
	RECY_CALL_BACK *finishwork;

	/*==================bellow must be reset=========================*/
	/* Setup error flags so they can be used directly. */
	int err;
	char errstr[128];
	/*cmd dispose*/
	int control;
	union {
		struct http_parse_info http_info;
		struct redis_parse_info redis_info;
	}parse;
	/* Regular command callbacks */
	struct command_list replies;
};

struct async_ctx *async_connect(const char *host, int port);
int async_set_link_cb(struct async_ctx *ac, LINK_CALL_BACK *getconnect, LINK_CALL_BACK *disconnect);
int async_set_recy_cb(struct async_ctx *ac, RECY_CALL_BACK *fcb, void *privdata);
//int async_initial(struct async_ctx *ac);
int async_command(struct async_ctx *ac, ASYNC_CALL_BACK fcb, void *privdata, const char *data, size_t size);
int async_startup(struct async_ctx *ac, enum proto_type ptype);
int async_distory(struct async_ctx *ac);

/* Handle read/write events */
void _async_handle_send(struct async_ctx *ac);
void _async_handle_recv(struct async_ctx *ac);


