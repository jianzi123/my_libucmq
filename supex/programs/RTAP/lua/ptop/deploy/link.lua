module("link")

OWN_POOL = {
	redis = {
                private = {
			--host = '192.168.1.3',
			--host = '127.0.0.1',
			host = '192.168.1.11',
			port = 6379,
		},
                driview = {
                        host = '192.168.1.11',
                        port = 6379,
                },
		public = {
			host = '192.168.1.11',
			--host = '192.168.1.11',
			port = 6379,
		},
		mapGridOnePercent = {
			host = '192.168.1.11',
			port = 6379,
			--host = '127.0.0.1',
			--port = 6379,
		},
		mapGridOnePercent01 = {
			host = '192.168.1.11',
			port = 6379,
		},
		dataCoreRedis = {
			host = '192.168.1.11',
			--host = '127.0.0.1',
			port = 6379,
		}
        },
	mysql = {},
}

OWN_DIED = {
        --[[
        redis = {
                private = {
			--host = '192.168.1.3',
			--host = '127.0.0.1',
			host = '192.168.1.11',
			port = 6379,
		},
		public = {
			host = '192.168.1.11',
			--host = '192.168.1.11',
			port = 6379,
		},
		mapGridOnePercent = {
			host = '192.168.1.11',
			port = 6379,
			--host = '127.0.0.1',
			--port = 6379,
		},
		mapGridOnePercent01 = {
			host = '192.168.1.11',
			port = 6379,
		},
		dataCoreRedis = {
			host = '192.168.1.11',
			--host = '127.0.0.1',
			port = 6379,
		}
        },
        ]]--
	mysql = {},
	http = {
		["weiboapi/v2/sendMultimediaPersonalWeibo"] = {
			host = "192.168.1.207",
			port = 80,
		},
		["dfsapi/v2/txt2voice"] = {
			--host = "api.daoke.io",
			--port = 80,
			host = "192.168.1.207",
			port = 80,
		},
		["spx_txt_to_voice"] = {
			host = "192.168.1.194",
			port = 2222,
		},
		--[[
		["getVoiceByTagName"] = {
		host = "221.228.231.88",
		port = 3005,
		},
		--]]
		["getVoiceByTagName"] = {
			host = "feeding.daoke.me/getVoiceByTagName",
			port = 80,
		},
		["getVoiceByTagID"] = {
			host = "221.228.231.88",
			port = 3005,
		},
		["dianping_server"] = {
			host = 'api.dianping.com',
			port = 80,
		},
		["/customizationapp/v2/callbackFetch4MilesAheadPoi"] = {
			host = "127.0.0.1",
			port = 8088,
		},
		["/customizationapp/v2/callbackDrivingPatternRemind"] = {
			host = "127.0.0.1",
			port = 8088,
		},
		["4milesAMR"] = {
			host = "127.0.0.1",
			port = 80,
		},
		["overSpeedAMR"] = {
			host = "127.0.0.1",
			port = 80,
		},
		["/roadRankapi/v1/getFrontSpeedByLBS"] = {
			host = "192.168.11.135",
			port = 8088,
		},
		["/getCityWeatherAlert"] = {
			host = "192.168.11.99",
			port = 3000,
		},
		["/getOilPrice"] = {
			host = "192.168.11.99",
			port = 3000,
		},
	},
}
