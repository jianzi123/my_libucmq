module("link")

OWN_POOL = {
	redis = {
		localRedis = {
			host = '127.0.0.1',
			port = 5080,
		},
		mapRTMileage = {
			host = '172.16.21.171',
			port = 5081,
		},
		mapDrimode= {
                        host = '192.168.11.142',
                        port = 5082,
                },
	},
	mysql = {
	},
}


OWN_DIED = {
	redis = {
	},
	mysql = {},
	http ={
                  ["weiboapi/v2/sendMultimediaPersonalWeibo"] = { host = "172.16.21.106",  port = 80 },

	},
}



