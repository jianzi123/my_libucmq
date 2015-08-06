#include "weibo.h"
#include "errors.h"
#include "check_parments.h"
#include "weibo_cfg.h"
#include "async_api.h"
#include "pool_api.h"
#include <string.h>

extern struct weibo_cfg_file g_weibo_cfg_file;
char weibo_http_format[] = "POST /%s HTTP/1.1\r\n" 
     "User-Agent: curl/7.33.0\r\n" 
     "Host: %s:%d\r\n" 
     "Content-Type: application/json; charset=utf-8\r\n"
     "Connection:%s\r\n" 
     "Content-Length:%d\r\n" 
     "Accept: */*\r\n\r\n%s";

//void http_callback(struct async_ctx *task, void *reply, void *data)
void http_callback(struct async_ctx *ac, void *reply, void *data)
{
	x_printf(D, "-------------\n");
	if (reply){
		struct net_cache *cache = reply;
		x_printf(D, "%s\n", cache->buf_addr);
	}
}

int parse_parments(Weibo *s, cJSON *obj)
{
	cJSON *temp;
	int ret = 0;

	/*appKey*/
	temp = cJSON_GetObjectItem(obj, "appKey");
	if (!temp){
		return API_INVALID_APPKEY;
	}
	s->appKey = temp->valuestring;

	/*multimediaURL*/
	temp = cJSON_GetObjectItem(obj, "multimediaURL");
	if (!temp){
		return API_INVALID_MULTIMEDIAURL;
	}else{
		s->multimediaURL = temp->valuestring;
		if( !s->multimediaURL ){
			return API_INVALID_MULTIMEDIAURL;
		}else{
			ret = upper_to_lower(s->multimediaURL);//将大写字母改成小写
			if (ret < 0){
				return API_INVALID_MULTIMEDIAURL;
			}else{
				if((!strstr(s->multimediaURL, "http://"))||(strlen(s->multimediaURL)>255)||
					(!strstr(s->multimediaURL,".amr") && !strstr(s->multimediaURL,".wav"))){
					return API_INVALID_MULTIMEDIAURL;
				}
			}
		}
	}
	/*sourceType*/
	temp = cJSON_GetObjectItem(obj, "sourceType");
	if (!temp){
		cJSON_AddNumberToObject(obj, "sourceType", 1);
	}
	
	/*interval*/
	temp = cJSON_GetObjectItem(obj, "interval");
	if (!temp){
		return API_INVALID_INTERVAL;
	}
	s->interval = temp->valueint;
	if( !s->interval||
			  s->interval < 0 ){
		return API_INVALID_INTERVAL;
	}

	/*endTime*/
	s->endTime = s->interval+time(0);
	cJSON_AddNumberToObject(obj, "endTime", s->endTime);
	/*sourceID*/
	temp = cJSON_GetObjectItem(obj, "sourceID");
	if (!temp){
		s->sourceID = "";
	}else{
		s->sourceID = temp->valuestring;
		if(!s->sourceID ||
				32 < strlen(s->sourceID)){
			return API_INVALID_SOURCEID;
		}
	}
	/*commentID*/
	temp = cJSON_GetObjectItem(obj, "commentID");
	if (!temp){
		s->commentID = "";
	}else{
		s->commentID = temp->valuestring;
		if( !s->commentID ||
				32 < strlen(s->commentID) ){
			return API_INVALID_COMMENTID;
		}
	}

	/*content*/
	char content[256] = {};
	temp = cJSON_GetObjectItem(obj, "content");
	if (!temp){
		s->content = "";
	}else{
		if (WEIBO_FAILED == content_decode((u_char *)content, (u_char *)temp->valuestring)){
			return API_INVALID_CONTENT;
		}
		cJSON *item = cJSON_CreateString( content );
		if (!item){
			return JSON_OP_FAIL;
		}
		cJSON_ReplaceItemInObject(obj, "content", item);
		s->content = item->valuestring;
	}


	/*level*/
	temp = cJSON_GetObjectItem(obj, "level");
	if (!temp){
		s->level = 99;
		cJSON_AddNumberToObject(obj, "level", 99);
	}else{
		s->level = temp->valueint;
		if (s->level > 99 || s->level < 0){
			return API_INVALID_LEVEL;
		}
	}

	/*tokenCode*/
	temp  = cJSON_GetObjectItem(obj, "tokenCode");
	if(!temp){
		s->tokenCode = "";
	}else{
		s->tokenCode = temp->valuestring;
	}

	/*receiverLongitude*/
	temp = cJSON_GetObjectItem(obj, "receiverLongitude");
	if (!temp){
		s->longitude = 	-1;
	}else{
		s->longitude = temp->valuedouble;
		if(s->longitude>180 || s->longitude<-180){
			return API_INVALID_RECEIVERLONGITUDE;
		}
	}

	/*receiverLatitude*/
	temp = cJSON_GetObjectItem(obj, "receiverLatitude");
	if (!temp){
		s->latitude = -1;
	}else{
		s->latitude = temp->valuedouble;
		if(s->latitude>90 || s->latitude<-90){
			return API_INVALID_RECEIVERLATITUDE;
		}
	}

	/*receiverDistance*/
	temp = cJSON_GetObjectItem(obj, "receiverDistance");
	if (!temp){
		s->distance = -1;
	}else{
		s->distance = temp->valuedouble;
		if (s->distance < 0){
			return API_INVALID_RECEIVERDISTANCE;
		}
	}

	/*receiverDirection*/
	temp = cJSON_GetObjectItem(obj, "receiverDirection");
	if (!temp){
		s->direction = -1;
		s->direction_deviation = -1;
	}else{
		cJSON *son = cJSON_GetArrayItem(temp, 0);
		if (!son){
			return API_INVALID_RECEIVERDIRECTION;
		}
		s->direction = son->valueint;
		if(s->direction > 360 ||
				(s->direction < 0 && s->direction != -1)){
			return API_INVALID_RECEIVERDIRECTION;
		}
		son = cJSON_GetArrayItem(temp, 1);
		if (son){
			s->direction_deviation = son->valueint;
			if(s->direction_deviation < 0 || s->direction_deviation > 180){
				return API_INVALID_RECEIVERDIRECTION;
			}
		}
	}

	/*receiverSpeed*/
	temp = cJSON_GetObjectItem(obj, "receiverSpeed");
	if (!temp){
		s->speed = -1;
		s->speed_deviation = -1;
	}else{
		cJSON *son = cJSON_GetArrayItem(temp, 0);
		if (!son){
			return API_INVALID_RECEIVERSPEED;
		}
		s->speed = son->valueint;
		if(s->speed < 0){
			return API_INVALID_RECEIVERSPEED;
		}
		son = cJSON_GetArrayItem(temp, 1);
		if (!son){
			s->speed_deviation = 999;
		}else{
			s->speed_deviation = son->valueint;
			if(s->speed_deviation < 0){
				return API_INVALID_RECEIVERSPEED;
			}
		}
	}

	/*invalidDis*/
	temp = cJSON_GetObjectItem(obj, "invalidDis");
	if (!temp){
		s->invalidDis = 0;
	}else{
		s->invalidDis = temp->valueint;
		if(s->invalidDis < 0){
			return API_INVALID_INVALIDDIS;
		}
	}
	/*senderAccountID*/
	temp = cJSON_GetObjectItem(obj, "senderAccountID");
	if (!temp){
		s->senderAccountID = "";
	}else{
		s->senderAccountID = temp->valuestring;
		if ( WEIBO_FAILED == check_user(s->senderAccountID) ){
			return API_INVALID_SENDERACCOUNTID;
		}
	}

	/*tipType*/
	temp = cJSON_GetObjectItem(obj, "tipType");
	if (!temp){
		s->tipType = 0;
		cJSON_AddNumberToObject(obj, "tipType", 0);
	}else{
		short type = s->tipType = temp->valueint;
		if( !(type==1 || type==0) ){
			return API_INVALID_TIPTYPE;
		}
	}

	/*callbackURL*/
	temp = cJSON_GetObjectItem(obj, "callbackURL");
	if (!temp){
		s->callbackURL = "";
	}else{
		s->callbackURL = temp->valuestring;
		if( !s->callbackURL){
			return API_INVALID_CALLBACKURL;
		}else{
			ret = upper_to_lower(s->callbackURL);
			if(ret <0 ){
				return API_INVALID_CALLBACKURL;
			}else{
				if((!strstr(s->multimediaURL, "http://"))||(strlen(s->multimediaURL)>255)){
				return API_INVALID_CALLBACKURL;
			}
		}
	}
}
	/*autoReply*/
	temp = cJSON_GetObjectItem(obj, "autoReply");
	if (!temp){
		s->autoReply = 0;
		cJSON_AddNumberToObject(obj, "autoReply", 0);
	}else{
		s->autoReply = temp->valueint;
		if( !(s->autoReply==1 || s->autoReply==0) ){
			return API_INVALID_AUTOREPLY;
		}
	}
	/*POIID*/
	temp = cJSON_GetObjectItem(obj, "POIID");

	if (!temp){
		s->POIID = "";
	}else{
		s->POIID = temp->valuestring;
		if( !s->POIID)
			return API_INVALID_POIID;
	}

	/*POIType*/
	temp = cJSON_GetObjectItem(obj, "POIType");

	if (!temp){
		s->POIType = "";
	}else{
		s->POIType = temp->valuestring;
		if( !s->POIType)
			return API_INVALID_POITYPE;
	}
	/*just for personal weibo*/
	if ((!s->groupID) && (!s->regionCode)){
		
		/*create bizid*/
		char bizid[50] = {};
		temp = cJSON_GetObjectItem(obj, "geometryType");
		if (!temp){
			strcat(bizid, "a4");//a4 个人微博
		}else{
			short type = s->geometryType = temp->valueint;
			if(type==4){
				strcat(bizid, "a6");//区域微博
			}
			else{
				if(!(type==1||type==2||type==3||type==5)){
					return API_INVALID_GEOMETRYTYPE;
				}
				strcat(bizid, "a8");//共享微博
			}
		}
		create_uuid(bizid);
		cJSON *item = cJSON_CreateString( bizid );
		if (!item){
			return JSON_OP_FAIL;
		}
		s->bizid = item->valuestring;
		cJSON_AddItemToObject(obj, "bizid", item);

		/*messageType*/
		temp = cJSON_GetObjectItem(obj, "messageType");
		if (!temp){
			if (s->commentID){
				s->messageType = 4;
				cJSON_AddNumberToObject(obj, "messageType", 4);
			}else{
				s->messageType =1;
				cJSON_AddNumberToObject(obj, "messageType", 1);
			
			} 
		}else{
			s->messageType = temp->valueint;
		}

		if(s->messageType !=1 && s->messageType !=4 ){
			return API_INVALID_MESSAGETYPE;
		}
		/*receiverAccountID*/

		temp = cJSON_GetObjectItem(obj, "receiverAccountID");
		if (!temp){
			return API_INVALID_RECEIVERACCOUNTID;
		}else{
			s->receiverAccountID = temp->valuestring;
			if ( WEIBO_FAILED == check_user(s->receiverAccountID) ){
				return API_INVALID_RECEIVERACCOUNTID;
			}
		}
	}

	/*just for group weibo*/
	/*receiveCrowd*/
	if (s->groupID){ 
		
		/*messageType*/
		temp = cJSON_GetObjectItem(obj, "messageType");
		if (!temp){
			s->messageType =3;
			cJSON_AddNumberToObject(obj, "messageType", 3);
		}else{
			s->messageType = temp->valueint;
			if (s->messageType == 1){
				s->messageType =2;
				cJSON *item = cJSON_CreateNumber( s->messageType );
				if (!item){
					return JSON_OP_FAIL;
				}
				cJSON_ReplaceItemInObject(obj, "messageType", item);
			}
		}

		if(s->messageType != 2 && s->messageType !=3 ){
			return API_INVALID_MESSAGETYPE;
		}

		temp = cJSON_GetObjectItem(obj, "receiveCrowd");
		if (temp){
			int num = cJSON_GetArraySize(temp);
			cJSON * son = cJSON_GetArrayItem(temp,0);
			if (son) {
				if (num > MAX_MEMBER || !son->valuestring){
					return API_INVALID_RECEIVECROWD;
				}
			}
		}
	/*applyCallbackURL*/
	temp = cJSON_GetObjectItem(obj, "applyCallbackURL");
	if (!temp){
		s->applyCallbackURL = NULL;
	}else{
		s->applyCallbackURL = temp->valuestring;
		if( !s->applyCallbackURL){
			return API_INVALID_APPLYCALLBACKURL;
		}else{
			ret = upper_to_lower(s->applyCallbackURL);
			if (ret <0 ){
				return API_INVALID_APPLYCALLBACKURL;
			}else{
				if(!strstr(s->applyCallbackURL, "http://")){
					return API_INVALID_APPLYCALLBACKURL;
				}
			}
		}
	}
	
	/*appendFileURL*/
	temp = cJSON_GetObjectItem(obj, "appendFileURL");
	if (!temp){
		s->appendFileURL = "";
	}else{
		s->appendFileURL = temp->valuestring;
		if( !s->appendFileURL||strlen(s->appendFileURL)>255){
			return API_INVALID_APPENDFILEURL;
		}
	}
	/*bizid*/
	temp = cJSON_GetObjectItem(obj, "bizid");
	if (!temp){
				return API_INVALID_BIZID;
		}else{
			s->bizid = temp->valuestring;
		}

		/*receiveSelf*/
		temp = cJSON_GetObjectItem(obj, "receiveSelf");
		if (!temp){
			s->receiveSelf = 1;
			cJSON_AddNumberToObject(obj, "receiveSelf", 1);
		}else{
			short  self = s->receiveSelf = temp->valueint;
			if( !(self==0 ||self ==1) ){
				return API_INVALID_RECEIVESELF;
			}
		}

		/*isOnline*/ 
		/*区分是给在线还是非在线发集团微博*/
		temp = cJSON_GetObjectItem(obj, "isOnline");
		if (!temp){
			s->isOnline = 1;
		}else{
			s->isOnline = temp->valueint;
			if(s->isOnline != 0 && s->isOnline != 1){
				return API_INVALID_ISONLINE;
			}
		}
	
		/*isChannel*/
		/*区分是给集团在线成员还是频道在线成员发集团微博*/
		temp = cJSON_GetObjectItem(obj, "isChannel");
		if (!temp){
			s->isChannel = 1;
		}else{
			s->isChannel = temp->valueint;
			if(s->isChannel != 0 &&s->isChannel != 1){
				return API_INVALID_ISCHANNEL;
			}
		}
	
	}

	if(s->regionCode)
	{
		
		/*messageType*/
		temp = cJSON_GetObjectItem(obj, "messageType");
		if (!temp){
			s->messageType =1;
		}

		temp = cJSON_GetObjectItem(obj, "receiveCrowd");
		if (temp){
			int num = cJSON_GetArraySize(temp);
			 cJSON * son = cJSON_GetArrayItem(temp,0);
			 if(son){	
				 if (num > MAX_MEMBER||!son->valuestring){
					 return API_INVALID_RECEIVECROWD;
				 }
			 }
		}
	/*applyCallbackURL*/
	temp = cJSON_GetObjectItem(obj, "applyCallbackURL");
	if (!temp){
		s->applyCallbackURL = NULL;
	}else{
		s->applyCallbackURL = temp->valuestring;
		if( !s->applyCallbackURL){
			return API_INVALID_APPLYCALLBACKURL;
		}else{
			ret = upper_to_lower(s->applyCallbackURL);
			if (ret <0 ){
				return API_INVALID_APPLYCALLBACKURL;
			}else{
				if(!strstr(s->applyCallbackURL, "http://")){
					return API_INVALID_APPLYCALLBACKURL;
				}
			}
		}
	}
	
	/*appendFileURL*/
	temp = cJSON_GetObjectItem(obj, "appendFileURL");
	if (!temp){
		s->appendFileURL = "";
	}else{
		s->appendFileURL = temp->valuestring;
		if( !s->appendFileURL||strlen(s->appendFileURL)>255){
			return API_INVALID_APPENDFILEURL;
		}
	}
	/*bizid*/
	temp = cJSON_GetObjectItem(obj, "bizid");
	if (!temp){
				return API_INVALID_BIZID;
		}else{
			s->bizid = temp->valuestring;
		}

		/*receiveSelf*/
		temp = cJSON_GetObjectItem(obj, "receiveSelf");
		if (!temp){
			s->receiveSelf = 1;
			cJSON_AddNumberToObject(obj, "receiveSelf", 1);
		}else{
			short  self = s->receiveSelf = temp->valueint;
			if( !(self==0 ||self ==1) ){
				return API_INVALID_RECEIVESELF;
			}
		}
		
	}

	return API_OK;

}
/*当有多台redis服务器时，当有请求时，可以时机选择服务器*/
int get_redis_link_index(void)
{
#if 0
	return random()%g_weibo_cfg_file.redis_count;//随机分配任务
#else
	static int idx = 0;//严格分配任务
	return (idx++)%g_weibo_cfg_file.weibo_count;
#endif
}

int get_redis_static_link_index(void)
{
#if 0
	return random()%g_weibo_cfg_file.redis_count;//随机分配任务
#else
	static int idx = 0;//严格分配任务
	return (idx++)%g_weibo_cfg_file.static_count;
#endif
}
/*当有多台http服务器时，当有请求时，可以时机选择服务器*/
int get_weidb_link_index(void)
{
#if 0
	return random()%g_weibo_cfg_file.redis_count;//随机分配任务
#else
	static int idx = 0;//严格分配任务
	return (idx++)%g_weibo_cfg_file.weidb_count;
#endif
}
void weibo_callback(char *url,int count)
{
	if(NULL == url) return;
    	char host[32];
	char path[128];
	short port   = 0;

	char buf[256];
	char body[32] ={};

	char *start  = NULL;
	char *end    = NULL;

	memset(host,0,32);
	memset(path,0,128);
	memset(buf,0,256);
	start = strstr(url,"://");
	if (start == NULL)
		return ;
	start = start +3;
	end = strchr(start,':');
	if (NULL == end){
		port  = 80 ;
		end = strchr(start,'/');
		strncpy(host,start,end-start);
		}else{
			strncpy(host,start,end-start);
			start = end + 1 ;
			end = strchr(start,'/');
			strncpy(buf,start,end-start);
			port = atoi(buf);
		}
	strcpy(path,end+1);
	sprintf(body,"count=%d",count);
	memset(buf,0,256);
	snprintf(buf,255,weibo_http_format,path,host,port,"Close",strlen(body), body);
	sync_tcp_ask(host, port, buf, strlen(buf), NULL,0);
	
	return ;
}

/*
 	ZADD accountID:weibo level..当前时间  bizid
	设置个人微博等级，ZADD 是redis自带的一个排序命令
	它会将bizid 按level..当前时间,从小到大排序,此次修改
	这边没动

*/

int weibo_set_priority(char *str, Weibo *src,struct ev_loop *loop,void *redis_priority,struct cnt_pool *redis_pool)
{
#if 0
	int ok = REDIS_OK;

	ok = redisAsyncCommand(redis_priority, default_callback,redis_pool,
			"ZADD %s:weiboPriority %02d%ld %s",
			str, src->level,time(0), src->bizid);
	if(REDIS_OK != ok){
		return REDIS_OP_FAIL;
	}

	return API_OK;
#endif


        int ok = REDIS_OK;
        char *proto;

        ok = cmd_to_proto(&proto, "ZADD %s:weiboPriority %02d%ld %s",
                          str, src->level,time(0), src->bizid);
        
	ok = async_command(redis_priority, default_callback,redis_pool, proto, strlen(proto));
        free(proto);
        if (ok == -1) {
                return REDIS_OP_FAIL;
        }

	if(REDIS_OK != ok){
                
		return REDIS_OP_FAIL;
	}

        ok = async_startup(redis_priority, PROTO_TYPE_REDIS);
        if (REDIS_OK != ok) {
                return REDIS_OP_FAIL;
        }
        
	return API_OK;
}
/*
 * 之前获得集团所有成员，通过MySQL app_weibo 中userGroupInfo查询得到
 *考虑到通道的实时，高效，所以将查询数据库改成从redis中获得集团成员
 *同时增加两个KEY groupID:memebers,groupID:onlineMemebers
 */
int weibo_set_receive(Weibo *src,struct ev_loop *loop,void *redis_receive,struct cnt_pool *redis_pool)
{
#if 0
	int ok = REDIS_OK;
	char format_array[4][32] = {
		"SMEMBERS %s:members", //集团成员
		"SMEMBERS %s:onlineMembers",//集团在线成员
		"SMEMBERS %s:channelOnlineUser",//频道在线成员
		"SMEMBERS %s:cityOnlineUser"};//城市在线成员
	redisAsyncContext *redis=(redisAsyncContext *)redis_receive;
	redis->data = src;
	
	/*0：查找所有集团成员，1：查找所有集团在线成员,2 查找频道在线成员,3城市在线成员 默认查找频道在线成员.*/
	if(src->groupID){
		int idx = (src->isChannel == 1)? 2: (src->isOnline ==1 ? 1 : 0);
		ok = redisAsyncCommand(redis_receive, get_callback_data, redis_pool,
				format_array[idx],
				src->groupID);
		if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
		}
	}
	else{
		int idx = 3;
		ok = redisAsyncCommand(redis_receive, get_callback_data, redis_pool,
				format_array[idx],
				src->regionCode);
		if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
		}
	}

	
	return API_OK;
#endif


	int ok = REDIS_OK;
        char *proto;
	char format_array[4][32] = {
		"SMEMBERS %s:members", //集团成员
		"SMEMBERS %s:onlineMembers",//集团在线成员
		"SMEMBERS %s:channelOnlineUser",//频道在线成员
		"SMEMBERS %s:cityOnlineUser"};//城市在线成员
	struct async_ctx *redis=(struct async_ctx *)redis_receive;
        
//	redis->data = src;
        redis->with_data = src;
	
	/*0：查找所有集团成员，1：查找所有集团在线成员,2 查找频道在线成员,3城市在线成员 默认查找频道在线成员.*/
	if(src->groupID){
		int idx = (src->isChannel == 1)? 2: (src->isOnline ==1 ? 1 : 0);
                
                ok = cmd_to_proto(&proto, format_array, src->groupID);
                if (ok == REDIS_ERR) {
                        return REDIS_OP_FAIL;
                }

                ok = async_command(redis_receive, get_callback_data, redis_pool, proto, strlen(proto));
                free(proto);
		if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
		}
                
                ok = async_startup(redis_receive, PROTO_TYPE_REDIS);
                if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
		}
	}
	else{
		int idx = 3;
                
                ok = cmd_to_proto(&proto, format_array, src->groupID);
                if (ok == REDIS_ERR) {
                        return REDIS_OP_FAIL;
                }

		ok = async_command(redis_receive, get_callback_data, redis_pool, proto, strlen(proto));
                free(proto);
		if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
		}
                
                ok = async_startup(redis_receive, PROTO_TYPE_REDIS);
                if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
		}
	}
	
	return API_OK;
}
/* 
 *之前版本是：将需要存储的数据重新组装，再经过json编码，通过set方式存入redis中
 *为了后面取数据时，不再json解码，所有这次数据采取Hash存储，通过Hmset方式存入redis中
*/

int weibo_send_info(Weibo *src, struct ev_loop *loop,void *redis_weibo,void *redis_senderInfo,void *redis_expire,struct cnt_pool *redis_pool)
{
#if 0
	int ok = REDIS_OK;

//集团微博比个人多一个filed groupID
	if (src->groupID){
		ok = redisAsyncCommand(redis_weibo, default_callback, redis_pool,
			"HMSET %s:weibo groupID %s multimediaFileURL %s endTime %ld tokenCode %s content %s direction [%d,%d]"
			" speed [%d,%d] level %d messageType %d tipType %d autoReply %d invalidDis %d"
			" longitude %f latitude %f distance %f ", 
			src->bizid,src->groupID, src->multimediaURL, src->endTime, src->tokenCode, src->content, src->direction, src->direction_deviation,
			src->speed, src->speed_deviation, src->level, src->messageType, src->tipType, src->autoReply, src->invalidDis,
			src->longitude, src->latitude, src->distance);
		
	if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
		}
	}else{
 //设置 bizid:weibo 
	ok = redisAsyncCommand(redis_weibo, default_callback, redis_pool,
			"HMSET %s:weibo multimediaFileURL %s endTime %ld tokenCode %s content %s direction [%d,%d]"
			" speed [%d,%d] level %d messageType %d tipType %d autoReply %d invalidDis %d"
			" longitude %f latitude %f distance %f POIID %s POIType %s", 
			src->bizid, src->multimediaURL, src->endTime, src->tokenCode, src->content, src->direction, src->direction_deviation,
			src->speed, src->speed_deviation, src->level, src->messageType, src->tipType, src->autoReply, src->invalidDis,
			src->longitude, src->latitude, src->distance, src->POIID, src->POIType);
		
		if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
		}
	}
//设置 bizid:senderInfo
	ok = redisAsyncCommand(redis_senderInfo, default_callback, redis_pool,
			"HMSET %s:senderInfo senderAccountID %s callbackURL %s sourceID %s appKey %s commentID %s",
			src->bizid, src->senderAccountID, src->callbackURL, src->sourceID, src->appKey, src->commentID);
	if(REDIS_OK != ok){
		return REDIS_OP_FAIL;
	}
//设置bizid:senderInfo有效时间
	ok = redisAsyncCommand(redis_expire, default_callback, redis_pool,
			"EXPIRE %s:senderInfo %d", src->bizid, 300+(src->interval));
	if(REDIS_OK != ok){
		return REDIS_OP_FAIL;
	}


	return API_OK;
#endif

	int ok = REDIS_OK;
        char *proto;

//集团微博比个人多一个filed groupID
	if (src->groupID){
                ok = cmd_to_proto(&proto, "HMSET %s:weibo groupID %s multimediaFileURL %s endTime %ld tokenCode %s content %s direction [%d,%d]"
                                  " speed [%d,%d] level %d messageType %d tipType %d autoReply %d invalidDis %d"
                                  " longitude %f latitude %f distance %f ", 
                                  src->bizid,src->groupID, src->multimediaURL, src->endTime, src->tokenCode, src->content, src->direction, src->direction_deviation,
                                  src->speed, src->speed_deviation, src->level, src->messageType, src->tipType, src->autoReply, src->invalidDis,
                                  src->longitude, src->latitude, src->distance);
                if (ok == REDIS_ERR) {
                        return REDIS_OP_FAIL;
                }
		ok = async_command(redis_weibo, default_callback, redis_pool, proto, strlen(proto));
                free(proto);
                if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
                }
                
                ok = async_startup(redis_weibo, PROTO_TYPE_REDIS);
                if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
                }
	}else{
 //设置 bizid:weibo
                ok = cmd_to_proto(&proto, "HMSET %s:weibo multimediaFileURL %s endTime %ld tokenCode %s content %s direction [%d,%d]"
                                  " speed [%d,%d] level %d messageType %d tipType %d autoReply %d invalidDis %d"
                                  " longitude %f latitude %f distance %f POIID %s POIType %s", 
                                  src->bizid, src->multimediaURL, src->endTime, src->tokenCode, src->content, src->direction, src->direction_deviation,
                                  src->speed, src->speed_deviation, src->level, src->messageType, src->tipType, src->autoReply, src->invalidDis,
                                  src->longitude, src->latitude, src->distance, src->POIID, src->POIType);
                if (ok == REDIS_ERR) {
                        return REDIS_OP_FAIL;
                }
                ok = async_command(redis_weibo, default_callback, redis_pool, proto, strlen(proto));
                free(proto);
		if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
		}
                ok = async_startup(redis_weibo, PROTO_TYPE_REDIS);
                if(REDIS_OK != ok){
			return REDIS_OP_FAIL;
                }
	}

//设置 bizid:senderInfo
//        printf("src->bizid->%s, src->senderAccountID->%s, src->callbackURL->%s, src->sourceID->%s, src->appKey->%s, src->commentID->%s\n",
//               src->bizid, src->senderAccountID, src->callbackURL, src->sourceID, src->appKey, src->commentID);

        ok = cmd_to_proto(&proto, "HMSET %s:senderInfo senderAccountID %s callbackURL %s sourceID %s appKey %s commentID %s",
                          src->bizid, src->senderAccountID, src->callbackURL, src->sourceID, src->appKey, src->commentID);

        if (ok == REDIS_ERR) {
                        return REDIS_OP_FAIL;
        }
	ok = async_command(redis_senderInfo, default_callback, redis_pool, proto, strlen(proto));
        free(proto);
	if(REDIS_OK != ok){
		return REDIS_OP_FAIL;
	}
        ok = async_startup(redis_senderInfo, PROTO_TYPE_REDIS);
        if(REDIS_OK != ok){
                return REDIS_OP_FAIL;
        }

//设置bizid:senderInfo有效时间
        ok = cmd_to_proto(&proto, "EXPIRE %s:senderInfo %d", src->bizid, 300+(src->interval));
        if (ok == REDIS_ERR) {
                        return REDIS_OP_FAIL;
        }
	ok = async_command(redis_expire, default_callback, redis_pool, proto, strlen(proto));
        free(proto);
	if(REDIS_OK != ok){
		return REDIS_OP_FAIL;
	}
        ok = async_startup(redis_expire, PROTO_TYPE_REDIS);
        if(REDIS_OK != ok){
                return REDIS_OP_FAIL;
        }

	return API_OK;
}
/*功能：将接受人群成员与查询出来的集团成员经行比较，如果不属于集团成员的将不会给他发微博信息
 *同时如果他设定receiveSelf = 0,将不会发给自己
 */
int parse_callback_data(struct redisReply *redis,char **str,char *refuse,cJSON *receiver)
{
	cJSON *temp;
	int count = 0;
	int times = 0;
	int right_id =0;
	int num = cJSON_GetArraySize(receiver);
	
	while (count <num){
		redisReply **element = redis->element;//从新定义变量，切忌直接使用变量++
		temp = cJSON_GetArrayItem(receiver,count);
		if(NULL== (temp->valuestring)){
			break;
		}
		while(times < redis->elements){
			if (strcmp((*element)->str,temp->valuestring )==0){
				if(strcmp(temp->valuestring,refuse)==0)
					break;
				 *str = temp->valuestring;
				 str++;
				 right_id++;
				 break;
			}
			element++;
			times++;
		}
		count++;
		times = 0;
	}
	temp = NULL;
	return right_id;
}
/*之前版本的redis是同步的，可以直接接受返回值使用
 *而这次修改redis 调用是异步的，所以不能简单的直接接收返回值
 *所以这边引用返还值是放在回调函数里来引用的
 *功能：如果设置了接受人群，调用一个比对函数，得到正确接收人群,遍历正确接受人群，设置个人微博等级
 *如果没有设置了接受人群，直接遍历查询结果，设置个人微博等级
 *
 * */
int get_callback_data(redisAsyncContext *c, void *reply, void *privdata)
{

	Weibo *receiver =(Weibo *)c->data;
	cJSON *temp = cJSON_GetObjectItem(receiver->cjson,"receiveCrowd");
	
	int i = 0;
	int right_id = 0;
	int ok = API_OK;
	char *refuseAccountID = "";
	char *str[MAX_MEMBER] = {};
	redisReply *redis = reply;
	/*默认自己接受，默认值为1*/	
	if (receiver->receiveSelf ==0)
		refuseAccountID = receiver->senderAccountID;
	
	if(temp){
		if (reply){
			right_id = parse_callback_data(redis,str,refuseAccountID,temp);
		}
	}else{
		right_id = (int)redis->elements;	
	}

	if (c->err) {
		cnt_free ( (struct cnt_pool *)privdata, (uintptr_t **)&c );
		printf("%s:GET MEMEBER LIST ERROR\n",receiver->groupID);
	}else{
		redisLibevEvents *e = c->ev.data;
		struct ev_loop *loop = e->loop;
		if (e->reading) {
			e->reading = 0;
			ev_io_stop(loop, &e->rev);
		}
		if (e->writing) {
			e->writing = 0;
			ev_io_stop(loop, &e->wev);
		}

		cnt_push ( (struct cnt_pool *)privdata, (uintptr_t **)&c );
		/*设定了接受人群*/ 
		if(temp){
			while( i < right_id){
				ok= weibo_set_priority(str[i],receiver,receiver->loop,receiver->redisPriority,receiver->pool);
				if (ok != API_OK){
					weibo_callback(receiver->applyCallbackURL,i);
					ok = REDIS_OP_FAIL;
					goto ERROR;
				}
				i++;
			}
		}else{/*没有设定接受人群，直接从groupID:channelOlineUser获得接受人群*/
			redisReply **element = redis->element;//从新定义变量，切忌直接使用变量++
			while( i < right_id){
				ok= weibo_set_priority((*element)->str,receiver,receiver->loop,receiver->redisPriority,receiver->pool);
				if (ok != API_OK){
					weibo_callback(receiver->applyCallbackURL,i);
					ok = REDIS_OP_FAIL;
					goto ERROR;
				}
				i++;
				element++;
			}
		}
	}

	weibo_callback(receiver->applyCallbackURL,i);
	
	cJSON_Delete(receiver->cjson);

	if(receiver){
		free(receiver);
		receiver = NULL;
	}
	return ok;
ERROR:
	if(receiver->cjson){
		cJSON_Delete(receiver->cjson);
	}

	if(receiver){
		free(receiver);
		receiver = NULL;
	}
	return ok;
}

int weibo_data_transmit(char *body,char *weibo_type,struct ev_loop *loop,struct cnt_pool *http_pool,struct async_ctx *task,char *host,short port)
{
#if 0
	char buff[MAX_LEN_STRING] = {};

	/************组装HTTP头********/
	snprintf(buff,MAX_LEN_STRING-1,weibo_http_format,weibo_type,host,port,"Keep-Alive",strlen(body), body);

	/*send http require*/
	int ret = async_command(task, http_callback, http_pool, buff, strlen(buff));
	if(ret== ASYNC_ERR){
		return HTTP_OP_FAIL;
	}
	ret = async_startup(task);
	if(ret== ASYNC_ERR){
		return HTTP_OP_FAIL;
	}

	return API_OK;
#endif


        char buff[MAX_LEN_STRING] = {};

	/************组装HTTP头********/
	snprintf(buff,MAX_LEN_STRING-1,weibo_http_format,weibo_type,host,port,"Keep-Alive",strlen(body), body);

	/*send http require*/
	int ret = async_command(task, http_callback, http_pool, buff, strlen(buff));
	if(ret== ASYNC_ERR){
		return HTTP_OP_FAIL;
	}
	ret = async_startup(task, PROTO_TYPE_HTTP);
	if(ret== ASYNC_ERR){
		return HTTP_OP_FAIL;
	}

	return API_OK;
}
