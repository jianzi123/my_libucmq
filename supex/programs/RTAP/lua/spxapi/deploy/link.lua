module("link")

OWN_POOL = {
	redis = {},
	mysql = {},
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
		["dfsapi/v2/mp32voice"] = {
			host = "192.168.1.207",
			--host = "192.168.1.6",
			port = 80,
		},
	},
}
