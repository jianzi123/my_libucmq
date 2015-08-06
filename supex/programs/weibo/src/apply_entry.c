/*
owner:houyaqian
phone:13524867067
description: the function of the code is to deal with the personal and group weibo.
the save of weibo is divided into two parts,one is the redis's save,which is writted by C,
the other is MySQL's save,which is writted by LUA.the fields saved by redis are nedded to lssue right now,
this time we add some fileds in redis,such as tipType,intervalDis,autoReply,meanwhile,we also delete some fileds,fileID,
multimediaFile,startTime,checkTokenCode,In MySQL ,we add POIID,POIType
*/
#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include "cJSON.h"

#include "apply_entry.h"
#include "net_cache.h"
#include "weibo.h"
#include "errors.h"
#include "check_parments.h"
#include "async_api.h"
#include "pool_api.h"
#include "weibo_cfg.h"
#include "cbs.h"

extern struct weibo_cfg_file g_weibo_cfg_file;

int entry_personal_weibo( struct ev_loop *loop, char *data, size_t size,char *bizid )
{
	int ok = API_OK;
	char *body = NULL;
	cJSON *obj = NULL;

	struct cnt_pool *redis_pool = NULL;
	struct cnt_pool *weidb_pool = NULL;
	

        struct async_ctx *redis_priority;
        struct async_ctx *redis_weibo;
        struct async_ctx *redis_senderInfo;
        struct async_ctx *redis_expire;
	struct async_ctx *task = NULL; 

	Weibo wb = {};
	int i = get_redis_link_index();
	char *host = g_weibo_cfg_file.weibo_store[i].host;
	short port = g_weibo_cfg_file.weibo_store[i].port;

/*一次性取出程序执行所有连接数，如果连接数不够，将数据打印到LOG日志里，程序返回，打印错误码，
  这样做是为了防止当连接数到达连接池顶峰时，一直取不到连接，代码一直夯在哪里*/
//	redis_priority = redis_pool_gain(loop, &redis_pool,host,port);
        redis_priority = (struct async_ctx *)pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_priority)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

	redis_weibo = (struct async_ctx *)pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_weibo)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

        redis_senderInfo = (struct async_ctx *)pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_senderInfo)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

        redis_expire = (struct async_ctx *)pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_expire)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

	i = get_weidb_link_index();
	host = g_weibo_cfg_file.weidb[i].host;
	port = g_weibo_cfg_file.weidb[i].port;
	task = (struct async_ctx *)pool_api_gain(loop, &weidb_pool, host, port);
	if (NULL==task)
	{
		x_printf(W, "%s\n",data);
		ok = HTTP_POOL_GAIN_FAILED;
		return ok;
	}

	/****判断传入进来的参数******/

	if (NULL == data || 0 == size) {
		ok = API_NO_DATA;
		goto ERROR_PERSONAL;
	}else {
		if (size > MAX_LEN_STRING){
			ok = API_DATA_TOO_LONG;
			goto ERROR_PERSONAL;
		}
	}

	/*data cjson parse*/
	obj = cJSON_Parse (data);
	if (NULL == obj) {
		ok = API_INVALID_JSON;
		goto ERROR_PERSONAL;
	}

	/*check parments*/
	ok = parse_parments (&wb, obj);
	if (ok != API_OK){
		goto ERROR_PERSONAL;
	}

	strcpy(bizid,wb.bizid);//将生成的bizid,带回去

	/*receiverAccountID:weiboPriority*/
	ok = weibo_set_priority(wb.receiverAccountID,&wb,loop,redis_priority,redis_pool);
	if (ok != API_OK){
		goto ERROR_PERSONAL;
	}

	/*bizid:weibo and bizid:senderInfo*/
	ok = weibo_send_info(&wb, loop,redis_weibo,redis_senderInfo,redis_expire,redis_pool);
	if (ok != API_OK){
		goto ERROR_PERSONAL;
	}

	/*cJSON form turn into string form*/
	body = cJSON_Print(obj);
	if (!body){
		ok = JSON_OP_FAIL;
		goto ERROR_PERSONAL;
	}
	/*data transmitting*/
	ok = weibo_data_transmit(body,"weidb_personal",loop,weidb_pool,task,host,port);
	if (ok != API_OK){
		goto ERROR_PERSONAL;
	}

	free(body);
	cJSON_Delete(obj);
	
	return ok;

ERROR_PERSONAL:
	if (obj){
		cJSON_Delete(obj);
	}
	if(body){
		free(body);
	}
	return ok;
}

int entry_group_weibo( struct ev_loop *loop, char *data, size_t size )
{
	int  ok  = API_OK;
	char *body = NULL;
	cJSON *obj = NULL;
	cJSON *temp= NULL;
	
	struct cnt_pool *redis_pool = NULL;
	struct cnt_pool *static_pool = NULL;
	struct cnt_pool *weidb_pool = NULL;
	
	void *redis_priority = NULL;
	void *redis_receive = NULL;
	void *redis_weibo = NULL;
	void *redis_senderInfo = NULL;
	void *redis_expire = NULL;
	struct async_ctx *task = NULL; 
/*从配置文件中，得到IP，和port*/
	int i = get_redis_link_index();
	char *host = g_weibo_cfg_file.weibo_store[i].host;
	short port = g_weibo_cfg_file.weibo_store[i].port;

/*一次性取出程序执行所有连接数，如果连接数不够，将数据打印到LOG日志里，程序返回，打印错误码，
  这样做是为了防止当连接数到达连接池顶峰时，一直取不到连接，代码一直夯在哪里*/

//	redis_priority = redis_pool_gain(loop, &redis_pool,host,port);
        redis_priority = pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_priority)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}


//	redis_weibo = redis_pool_gain(loop, &redis_pool,host,port);
        redis_weibo = pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_weibo)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

//	redis_senderInfo = redis_pool_gain(loop, &redis_pool,host,port);
        redis_senderInfo = pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_senderInfo)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

//	redis_expire = redis_pool_gain(loop, &redis_pool,host,port);
        redis_expire = pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_expire)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

	
	i = get_redis_static_link_index();
	host = g_weibo_cfg_file.weibo_static[i].host;
	port = g_weibo_cfg_file.weibo_static[i].port;
//	redis_receive = redis_pool_gain(loop, &static_pool,host,port);
        redis_receive = pool_api_gain(loop, &static_pool,host,port);
	if (NULL==redis_receive)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

	i = get_weidb_link_index();
	host = g_weibo_cfg_file.weidb[i].host;
	port = g_weibo_cfg_file.weidb[i].port;
	task = (struct async_ctx *)pool_api_gain(loop, &weidb_pool, host, port);
	if (NULL==task)
	{
		x_printf(W, "%s\n",data);
		ok = HTTP_POOL_GAIN_FAILED;
		return ok;
	}
	Weibo *wb = calloc(1, sizeof(Weibo));//开空间并初始化
	if(!wb){
		ok = API_NO_MEMORY;
		goto ERROR_PUBLIC;	
	}
	wb->loop = loop;
	/******判断传入进来的参数******/
	if (NULL == data || 0 == size) {
		ok = API_NO_DATA;
		goto ERROR_PUBLIC;
	}else{
		if (size > MAX_LEN_STRING){
			ok = API_DATA_TOO_LONG;
			goto ERROR_PUBLIC;
		}
	}

	/*data cjson parse*/

	obj = cJSON_Parse (data);
	if (NULL == obj) {
		ok = API_INVALID_JSON;
		goto ERROR_PUBLIC;
	}
	wb->cjson = obj;

	/*check groupID*/
	temp = cJSON_GetObjectItem(obj, "groupID");
	if (NULL==temp){
		ok = API_INVALID_GROUPID;
		goto ERROR_PUBLIC;
	}
	wb->groupID = temp->valuestring;
	if (!wb->groupID){
		ok = API_INVALID_GROUPID;
		goto ERROR_PUBLIC;
	}
	/*check other*/
	ok = parse_parments (wb, obj);
	if (ok != API_OK){
		goto ERROR_PUBLIC;
	}
	wb->loop = loop;
	wb->pool = redis_pool;
	wb->redisPriority = redis_priority;

	/*receiver:priority */
	ok = weibo_set_receive(wb,loop,redis_receive,static_pool);
	if (ok != API_OK){
		goto ERROR_PUBLIC;
	}

	/*bizid:weibo and bizid:senderInfo*/
	ok = weibo_send_info(wb,loop,redis_weibo,redis_senderInfo,redis_expire,redis_pool);
	if (ok != API_OK){
		goto ERROR_PUBLIC;
	}

	/*cJSON form turn into string form*/
	body = cJSON_Print(obj);
	if (!body){
		ok = JSON_OP_FAIL;
		goto ERROR_PUBLIC;
	}
	
	/*data transmitting*/
	ok=weibo_data_transmit(body,"weidb_group",loop,weidb_pool,task,host,port);
	if (ok != API_OK){
		goto ERROR_PUBLIC;
	}
	free(body);
	/*cant't free json,free it in callback*/
	obj = NULL;
	return ok;
ERROR_PUBLIC:
	if (obj){
		cJSON_Delete(obj);
	}
	if(body){
		free(body);
	}
	return ok;

}

int entry_city_weibo(struct ev_loop *loop, char *data, size_t size)
{
	int  ok  = API_OK;
	char *body = NULL;
	cJSON *obj = NULL;
	cJSON *temp= NULL;
	
	struct cnt_pool *redis_pool = NULL;
	struct cnt_pool *static_pool = NULL;
	struct cnt_pool *weidb_pool = NULL;
	
	void *redis_priority = NULL;
	void *redis_receive = NULL;
	void *redis_weibo = NULL;
	void *redis_senderInfo = NULL;
	void *redis_expire = NULL;
	struct async_ctx *task = NULL; 
/*从配置文件中，得到IP，和port*/
	int i = get_redis_link_index();
	char *host = g_weibo_cfg_file.weibo_store[i].host;
	short port = g_weibo_cfg_file.weibo_store[i].port;

/*一次性取出程序执行所有连接数，如果连接数不够，将数据打印到LOG日志里，程序返回，打印错误码，
  这样做是为了防止当连接数到达连接池顶峰时，一直取不到连接，代码一直夯在哪里*/

//	redis_priority = redis_pool_gain(loop, &redis_pool,host,port);
        redis_priority = pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_priority)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}


//	redis_weibo = redis_pool_gain(loop, &redis_pool,host,port);
        redis_weibo = pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_weibo)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

//	redis_senderInfo = redis_pool_gain(loop, &redis_pool,host,port);
        redis_senderInfo = pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_senderInfo)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

//	redis_expire = redis_pool_gain(loop, &redis_pool,host,port);
        redis_expire = pool_api_gain(loop, &redis_pool,host,port);
	if (NULL==redis_expire)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}
	
	i = get_redis_static_link_index();
	host = g_weibo_cfg_file.weibo_static[i].host;
	port = g_weibo_cfg_file.weibo_static[i].port;
//	redis_receive = redis_pool_gain(loop, &static_pool,host,port);
        redis_receive = pool_api_gain(loop, &static_pool,host,port);
	if (NULL==redis_receive)
	{
		x_printf(W, "%s\n",data);
		ok = REDIS_POOL_GAIN_FAILED;
		return ok;
	}

	i = get_weidb_link_index();
	host = g_weibo_cfg_file.weidb[i].host;
	port = g_weibo_cfg_file.weidb[i].port;
	task = (struct async_ctx *)pool_api_gain(loop, &weidb_pool, host, port);
	if (NULL==task)
	{
		x_printf(W, "%s\n",data);
		ok = HTTP_POOL_GAIN_FAILED;
		return ok;
	}
	Weibo *wb = calloc(1, sizeof(Weibo));//开空间并初始化
	if(!wb){
		ok = API_NO_MEMORY;
		goto ERROR_CITY;	
	}
	wb->loop = loop;
	/******判断传入进来的参数******/
	if (NULL == data || 0 == size) {
		ok = API_NO_DATA;
		goto ERROR_CITY;
	}else{
		if (size > MAX_LEN_STRING){
			ok = API_DATA_TOO_LONG;
			goto ERROR_CITY;
		}
	}

	/*data cjson parse*/

	obj = cJSON_Parse (data);
	if (NULL == obj) {
		ok = API_INVALID_JSON;
		goto ERROR_CITY;
	}
	wb->cjson = obj;

	/*check regionCode*/
	temp = cJSON_GetObjectItem(obj, "regionCode");
	if (NULL==temp){
		ok = API_INVALID_REGIONCODE;
		goto ERROR_CITY;
	}
	wb->regionCode = temp->valuestring;
	if(!wb->regionCode){
		ok = API_INVALID_REGIONCODE;
		goto ERROR_CITY;
	}
	/*check other*/
	ok = parse_parments (wb, obj);
	if (ok != API_OK){
		goto ERROR_CITY;
	}
	wb->loop = loop;
	wb->pool = redis_pool;
	wb->redisPriority = redis_priority;

	/*receiver:priority */
	ok = weibo_set_receive(wb,loop,redis_receive,static_pool);
	if (ok != API_OK){
		goto ERROR_CITY;
	}

	/*bizid:weibo and bizid:senderInfo*/
	ok = weibo_send_info(wb,loop,redis_weibo,redis_senderInfo,redis_expire,redis_pool);
	if (ok != API_OK){
		goto ERROR_CITY;
	}

	/*cJSON form turn into string form*/
	body = cJSON_Print(obj);
	if (!body){
		ok = JSON_OP_FAIL;
		goto ERROR_CITY;
	}
	
	/*data transmitting*/
	ok=weibo_data_transmit(body,"weidb_city",loop,weidb_pool,task,host,port);
	if (ok != API_OK){
		goto ERROR_CITY;
	}
	free(body);
	/*cant't free json,free it in callback*/
	obj = NULL;
	return ok;
ERROR_CITY:
	if (obj){
		cJSON_Delete(obj);
	}
	if(body){
		free(body);
	}
	return ok;
	
}
