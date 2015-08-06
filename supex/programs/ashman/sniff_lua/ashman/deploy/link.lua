module("link")

OWN_POOL = {
	redis = {
		private = {
			host = '192.168.1.14',
			port = 6379,
		},
		statistic = {
		    	host = '192.168.1.11',
			port = 6379,
		},
		mapGPSData = {
			host = "192.168.1.11",
			port = 6379,
		},
		mapGridOnePercentV2 = {
			host = "192.168.1.11",
			port = 5072,
		}
	},
	mysql = {},
}

OWN_DIED = {
	redis = {},
	mysql = {},
	http = {
	    	dataCore= {
			host = "192.168.1.102",
			port = 8080,
		}
	},
}
