module("link")

OWN_POOL = {
	redis = {
		mapDrimode= {
			host = '127.0.0.1',
			port = 6379,
		},


	roadRelation = {
            host = '192.168.1.10',
            port =  4060,
        },
        mapRoadLine = {
            host = '192.168.1.10',
            port =  5602,
        },
        mapLineNode = {
            host = '192.168.1.10',
            port =  5601,
        },
        mapRoadInfo = {
            host = '192.168.1.9',
            port =  5603,
        },

	},
	mysql = {
	},
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {},

     http ={
                  ["weiboapi/v2/sendMultimediaPersonalWeibo"]             = { host = "192.168.1.68",  port = 8088 },
  
          },

}



