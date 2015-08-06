# pragma once 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>
#include <time.h>
#include <math.h>
#include "async_api.h"

#include "cbs.h"
#include "cJSON.h"

#define MAX_MEMBER 	50
#define MAX_LEN_STRING  2048

typedef struct weibo
{
	char   *multimediaURL,
	       *tokenCode,
	       *senderAccountID,
	       *receiverAccountID,
	       *callbackURL,
	       *appendFileURL,
	       *applyCallbackURL,
	       *sourceID,
	       *commentID,
	       *content,
	       *bizid,
	       *appKey,
	       *groupID,
	       *regionCode,
	       *POIID,
	       *POIType;

	short  level,
	       senderType,
	       geometryType,
	       messageType,
	       tipType,
	       autoReply,
	       direction,
	       direction_deviation,
	       speed,
	       speed_deviation,
	       interval,
	       isOnline,
	       isChannel,
	       receiveSelf;

	int 	invalidDis;
	long 	endTime;
	
	double longitude,
	       latitude,
	       distance;

	struct ev_loop *loop;
	cJSON *cjson;
	void *redisPriority;
	void *pool;

}Weibo;

void weibo_callback(char *p,int num);
int parse_parments(Weibo *s, cJSON *obj);
int weibo_set_receive(Weibo *src,struct ev_loop *loop,void *redis_receive,struct cnt_pool *redis_pool);
int weibo_set_priority(char *str, Weibo *src,struct ev_loop *loop,void *redis_priority,struct cnt_pool *redis_pool);
int weibo_send_info(Weibo *src, struct ev_loop *loop,void *redis_weibo,void *redis_senderInfo,void *redis_expire,struct cnt_pool *redis_pool);
int parse_callback_data(redisReply *redis,char **str,char *refuse,cJSON *receiver);
int get_callback_data(redisAsyncContext *c, void *r, void *privdata);
int weibo_data_transmit(char *body,char *weibo_type,struct ev_loop *loop,struct cnt_pool *http_pool,struct async_ctx *task,char *host,short port);
int get_redis_link_index(void);
int get_redis_static_link_index(void);
int get_weidb_link_index(void);


