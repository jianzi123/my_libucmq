module("link")

OWN_POOL = {
	redis = {
		public = {
			host = '192.168.1.11',
			port = 6379,
		},
		private = {
			host = '192.168.1.11',
			port = 6379,
		},
		weibo = {
			host = '192.168.1.11',
			port = 6379,
		},
		mapGridOnePercent = {
			host = '192.168.1.11',
			port = 6379,
		},
		statistic = {
			host = '192.168.1.11',
			port = 6379,
		},
		mapFrontPosition = {
			host = '192.168.1.11',
			port = 6379,
		},
	},

	mysql = {},

}


OWN_DIED = {
	api = {
		point_match_road = { host = "221.228.231.82", port = 80, path = 'v2/pointMatchRoad'},
		point_server    = { host = "192.168.1.3", port = 80, path = 'daokePoints' },
	},
	http ={
		["weiboapi/v2/sendMultimediaPersonalWeibo"] = { host = "192.168.1.68", port = 8088 },
		["appcenterApply.json"]			= { host = "127.0.0.1", port = 9999 },
		["dfsTxt2Voice"] = { 
			host = 'api.daoke.io', 
			port = 80,
		},
		["spx_txt_to_voice"] = { 
			host = "127.0.0.1",
			port = 2222,
		}, 
		---- 获取周围道客
		mapAroundDaoke				= { host = '127.0.0.1', port = 80 },
	},
}



