module("link")

OWN_POOL = {
	redis = {

        public = {
            host = '192.168.1.11',
            port = 6379,
        },
        private = {
            host = '192.168.1.14',
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
	mapLandMark = {
		host = '192.168.1.10',
		port = 5601
	},
		MapLineInRoad = {
			host = '192.168.1.11',
			port = 5605,
		},
		MapPersonalFrontPOI = {
			host = '192.168.1.9',
			port = 5231,
		},
		mapRoadReference = {
			host = '192.168.1.11',
			port = 5604,
		},
        
        mapGPSData = {
            host = '127.0.0.1',
            port = 6379,
        },			
	},
	mysql = {

        app_mirrtalk___config = {
            host = '192.168.1.12',
            port = 3307,
            database = 'config',
            user = 'observer',
            password ='abc123',
        },

	},
}


OWN_DIED = {
	redis = {
	},
	mysql = {},
	http = {
		["weiboapi/v2/sendMultimediaPersonalWeibo"] = {
			host = "192.168.1.3",
			port = 8088,
		},
		["dfsapi/v2/txt2voice"] = {
			host = "api.daoke.io",
			port = 80,
		},
        ["DataCore/autoGraph/addTravelRoadInfo"] = {
            host = "192.168.11.83",
            port = 8088,
        },
	},
}



