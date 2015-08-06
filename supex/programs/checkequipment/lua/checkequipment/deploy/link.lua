module("link")

OWN_POOL = {
	redis = {
	equipmentInfo = {
			host = '127.0.0.1',
			port = 6379,
		},
}
}


OWN_DIED = {
	redis = {},
	mysql = {},
	http = {},

     http ={
                  ["weiboapi/v2/sendMultimediaPersonalWeibo"]             = { host = "192.168.1.68",  port = 8088 },
  
          },

}



