module("link")

OWN_POOL = {
	redis = {

		public = {
			host = '172.16.21.44',
			port = 6349,
		},
		driview = {
                        host = '172.16.21.119',
                        port = 6410,
                },
                private = {
                        host = '172.16.21.41',
                        port = 6319,
                },
                private_hash={
                     --   {'M', '172.16.21.41', 6319, 30},
                        {'M', '172.16.21.134', 6400, 30},
                        {'M', '172.16.21.134', 6401, 30},
                        {'M', '172.16.21.134', 6402, 30},
                        {'M', '172.16.21.134', 6403, 30},
                        {'M', '172.16.21.158', 6404, 30},
                        {'M', '172.16.21.158', 6405, 30},
                        {'M', '172.16.21.158', 6406, 30},
                        {'M', '172.16.21.158', 6407, 30},
                },
		POIInfo = { 
			host = '172.16.21.112',
			port =  5241,
		},

		roadRelation = {
			host = '172.16.21.111',
			port =  4060,
		},
		mapRoadLine = {
			host = '172.16.21.54',
			port =  5602,
		},
		mapLineNode = {
			host = '172.16.21.53',
			port =  5601,
		},
		mapRoadInfo = {
			host = '172.16.21.55',
			port =  5603,
		},
		mapLandMark = {
			host = '172.16.21.52',
			port = 5531,
		},
		MapLineInRoad = {
			host = '172.16.21.52',
			port = 5605,
		},
		MapPersonalFrontPOI = {
			host = '172.16.21.47',
			port = 5231,
		},
		mapRoadReference = {
			host = '172.16.21.63',
			port = 5604,
		},

		mapGPSData = {
			host = '172.16.21.48',
			port = 5301,
		},

	},
	mysql = {

		app_mirrtalk___config = {
			host = '172.16.21.21',
			port = 3306,
			database = 'config',
			user = 'app_mirrtalk',
			password ='102cfgDK',
		},

	},
}


OWN_DIED = {
	redis = {
	},
	mysql = {},
	http = {
		["weiboapi/v2/sendMultimediaPersonalWeibo"] = {
			host = "172.16.21.106",
			port = 80,
		},
		["dfsapi/v2/txt2voice"] = {
			host = "172.16.21.176",
			port = 80,
		},
		
		["DataCore/autoGraph/addTravelRoadInfo"] = {
			host = "172.16.21.105",
			port = 9098,
		},
		["p2p_fetch_4_miles_ahead_poi"]          = { 
			host = "127.0.0.1", 
			port = 4080,
		},
	},
}
