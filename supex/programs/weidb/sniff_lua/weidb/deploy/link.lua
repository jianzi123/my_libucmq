module("link")

OWN_POOL = {
	redis = {
		weiboStore = {
			host = '127.0.0.1',
			port = 6379,
		},

		---- 使用1%的网格数据,运维请注意*** 2014-06-18 jiang z.s. 
			mapGridOnePercent = {
				host = '192.168.1.10',
				port = 5071,
			},

	},
	mysql = {

		app_roadmap___roadmap = {
			host = '192.168.1.6',
			port = 3306,
			database = 'roadMap',
			user = 'root',
			password ='Agh34GHHJklHDDdEAb',
		},
		app_mirrtalk___config = {
			host = '192.168.1.12',
			port = 3307,
			database = 'config',
			user = 'observer',
			password ='abc123',
		},
		app_weibo___weibo ={
			host = '192.168.1.6',
			port = 3306,
			database = 'app_weibo',
			user = 'root',
			password ='Agh34GHHJklHDDdEAb',
		},
		app_ns___newStatus = {
			host = '192.168.1.6',
			port = 3306,
			database = 'newStatus',
			user = 'app_ns',
			password ='DKnsMSG110',
		},

		app_weibo___weibo_read ={
			host = '192.168.1.6',
			port = 3306,
			database = 'app_new_weibo_read',
			user = 'root',
			password ='Agh34GHHJklHDDdEAb',
		},
	},
}

OWN_DIED = {
	redis = {},
	mysql = {},
	http = {},
}
