module("link")

OWN_POOL = {
	redis = {},
	mysql = {},
}


OWN_DIED = {
	redis = {},
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
	},
}



