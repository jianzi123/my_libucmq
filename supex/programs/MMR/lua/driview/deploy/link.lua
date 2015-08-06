module("link")

OWN_POOL = {
	redis = {
		public = {
			host = '192.168.1.11',
			port = 6379,
		},
		driview = {
			host = '192.168.1.11',
			port = 6379,
		},
		private = {
			host = '192.168.1.11',
			port = 6379,
		},
                private_hash={
                        {'M', '192.168.1.11', 6379, 30},
                        {'M', '192.168.1.11', 6379, 30},
                },
                Mileage = {
                        host = '192.168.1.11',
                        port = 6379,
                },
		weibo = {
			host = '192.168.1.11',
			port = 6379,
		},
		mapGridOnePercent = {
			--host = '192.168.1.10',
			--port = 5071,
			host = '192.168.1.11',
			port = 6379,
		},
		statistic = {
			host = '192.168.1.11',
			port = 6379,
		},
		mapFrontPosition = {
			--host = '192.168.1.9',
			--port = 5231,
			host = '192.168.1.11',
			port = 6379,
		},
		mapFrontPosition_new = {
			--host = '192.168.1.9',
			--port = 5231,
			host = '192.168.1.11',
			port = 6379,
		},
		roadRelation = {
			--host = '192.168.1.10',
			--port =  4060,
			host = '192.168.1.11',
			port = 6379,
		},  
		mapRoadLine = { 
			--host = '192.168.1.10',
			--port =  5602,
			host = '192.168.1.11',
			port = 6379,
		},  
		mapLineNode = { 
			--host = '192.168.1.10',
			--port =  5601,
			host = '192.168.1.11',
			port = 6379,
		},  
		mapRoadInfo = {
			--host = '192.168.1.9',
			--port = 5603,
			host = '192.168.1.11',
			port = 6379,
		},
		dataCoreRedis = {
			host = '192.168.1.11',
			--host = '127.0.0.1',
			port = 6379,
		},
		POIInfo = {
			host = '192.168.1.11',
			port =  6379,
		},
	},

}

OWN_DIED = {
	http ={
		["customizationapp/poweronDiaryStart"]                  = { host = "192.168.1.68",  port = 8088 },
		["weiboapi/v2/sendMultimediaPersonalWeibo"]             = { host = "192.168.1.68",  port = 8088 },
		["appcenterApply.json"]                                 = { host = "127.0.0.1",     port = 9999 },
		["robaisApply.json"]                                    = { host = "127.0.0.1",     port = 6666 },
		["p2p_traffic_remind"]                                  = { host = "127.0.0.1",     port = 7777 },
		["p2p_offsite_remind"]                                  = { host = "127.0.0.1",     port = 7777 },
		["/DataCore/autoGraph/getMonthMileageByAccount.do"]     = { host = "221.228.231.86", port = 9098},
		["p2p_mon_driving_mileage_remind"]                      = { host = "127.0.0.1", port = 7777 },
		["p2p_limit_speed_remind"]                              = { host = "127.0.0.1", port = 7777 },
		["p2p_driving_pattern_remind"]                          = { host = "127.0.0.1", port = 7777 },
		["p2p_continuous_driving_mileage_remind"]               = { host = "127.0.0.1", port = 7777 },
		["p2p_over_speed"]                                      = { host = "127.0.0.1", port = 7777 },
		["/DataCore/realTime/powerOff.do"]                      = {host = '192.168.1.102',  port = 8080},
		["p2p_weather_forcast"]                                 = { host = "127.0.0.1", port = 7777 },


		----正点时钟 nginxapi 用户谢尔值 2014-10-16 
		['crazyapi/v2/shareValueTask'] 				= {host = "127.0.0.1", port =  80 },

		----测试服务器URI(转发多台)
		['publicentry'] = { 
			{host = '127.0.0.1', port = 8999},
		},

	},
}
