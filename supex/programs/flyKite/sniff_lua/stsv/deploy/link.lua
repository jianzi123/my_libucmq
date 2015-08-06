module("link")

OWN_POOL = {
	redis = {
		private = {
			host = '192.168.1.14',
			port = 6379,
		},
		newStatus = {
			host = '192.168.1.12',
			port = 7777,
		},
		tsdb = {
			host = '192.168.1.14',
			port = 7502,
		},
		public = {
			host = '192.168.1.11',
			port = 6379,
		},
		roadRelation = {
			host = '192.168.1.10',
			port = 4060,
		},  
		--[[
		roadRelation = {
		host = '192.168.1.14',
		port =  5555,
		},  
		--]]
		mapRoadLine = { 
			host = '192.168.1.10',
			port = 5602,
		},  
		mapLineNode = { 
			host = '192.168.1.10',
			port = 5601,
		},  
		mapRoadInfo = {
			host = '192.168.1.9',
			port = 5603,
		},
		mapLandMark = { 
			host = '192.168.1.10',
			port = 5531,    
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
		private = {
			host = '192.168.1.11',
			port = 6379,
		},
		public = {
			host = '192.168.1.11',
			port = 6379,
		},
	},
	mysql = {},
	http = {
		["tsearchapi/v2/getgps"]={
			host = 'tsdb.mirrtalk.com',
			port = 80,
		},
		["DataCore/autoGraph/addTravelRoadInfo"] = {
			host = "192.168.1.13",
			port = 9098,
		},
	},
}
