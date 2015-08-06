#include "errors.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

struct weibo_error {
	int code;
	char *desc;
} g_error_list[] = {
	{ API_OK,                       		"Request OK",                 		 },
	{ API_FAILED,                       	        "Request FAILED",                 	 },
	{ API_ERR_BASE,                 		"ERROR BASE",                 		 },
	{ API_WEIBO_EXPIRE,             		"weibo expire",           		 },

	{ API_NO_MEMORY,                  		" no memory!",               		 },
	{ API_NO_DATA,                  		" no data!",                		 },
	{ API_DATA_TOO_LONG,                  		" input data is too long!",                     },
	{ API_INVALID_JSON,             		"json parse failed!",           		 },
	{ API_INVALID_APPKEY,           		"appkey is error!",         		 },
	{ API_INVALID_MULTIMEDIAURL,    		"multimediaURL is error!",  		 },
	{ API_INVALID_ENDTIME,          		"endTime is error!",        		 },
	{ API_INVALID_SOURCEID,         		"sourceID is error!",       		 },
	{ API_INVALID_COMMENTID,        		"commentID is errror!",      		 },
	{ API_INVALID_MESSAGETYPE,      		"messageType is error!",    		 },
	{ API_INVALID_SENDERTYPE,       		"senderType is error!",     		 },
	{ API_INVALID_CONTENT,          		"content is error!",        		 },
	{ API_INVALID_GEOMETRYTYPE,     		"geometryType is error !",   		 },
	{ API_INVALID_RECEIVERLONGITUDE,		"receiverLongitude is error!",	 },
	{ API_INVALID_RECEIVERLATITUDE, 		"receiverLatitude is error!",		 },
	{ API_INVALID_RECEIVERDISTANCE, 		"receiverDistance is error!",		 },
	{ API_INVALID_RECEIVERDIRECTION,		"receiverDirection is error!",	 },
	{ API_INVALID_RECEIVERSPEED,    		"receiverSpeed is error!",   		 },
	{ API_INVALID_INVALIDDIS,      		"invalidDis is error!",    		 },
	{ API_INVALID_BIZID,            		"bizid is error!",          		 },
	{ API_INVALID_INTERVAL,            		"interval is error!",                	 },
	{ API_INVALID_LEVEL,            		"level is error!",          		 },
	{ API_INVALID_RECEIVERACCOUNTID,		"receiverAccountID is error!",	 },
	{ API_INVALID_SENDERACCOUNTID,  		" senderAccountID is error!",		 },
	{ API_INVALID_TIPTYPE,          		"tipType is error!",                   },
	{ API_INVALID_CALLBACKURL,      		"callbackURL is error!",   		 },
	{ API_INVALID_APPLYCALLBACKURL,      	        "applyCallbackURL is error!",   	         },
	{ API_INVALID_APPENDFILEURL,      	        "appendFileURL is error!",   	         },
	{ API_INVALID_AUTOREPLY,        		"autoReply is error!",                 },
	{ API_INVALID_POIID,        			"POIID is error!",	                 },
	{ API_INVALID_POITYPE,        			"POIType is error!",  	         },
	{ API_INVALID_GROUPID,            		"groupID is error!",                   },
	{ API_INVALID_REGIONCODE,            		"regionCode is error!",                   },
	{ API_INVALID_RECEIVESELF,      		"receiveSelf is error!",               },
	{ API_INVALID_ISONLINE,				"isOnline is error!",	         },
	{ API_INVALID_ISCHANNEL,			"isChannel is erro1!",         },
	{ API_INVALID_RECEIVECROWD,			"receiveCrowd is error!",    	},

	{ JSON_OP_FAIL,                         "json failed!!",                       },
	{ HTTP_OP_FAIL,                         "http fail filed!",                       },
	{ REDIS_OP_FAIL,                        "redis failed!",                      },
	
	{ REDIS_POOL_GAIN_FAILED,                "redis pool gain failed",            },
	{ HTTP_POOL_GAIN_FAILED,                "redis pool gain failed",            },

	{ API_ERR_OVER,                 "ERROR OVER",                  },
};

char *handle_error(int err)
{
	switch (err) {
		case API_OK:
			break;
		case API_ERR_BASE ... API_ERR_OVER:
			x_printf(E, "%s\n", g_error_list[err].desc);
			break;
		default:
			x_printf(E, "Unknow error!\n");
	}
	return g_error_list[err].desc;
}

