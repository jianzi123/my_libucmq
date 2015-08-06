module("cfg")


keywords = {
}

workfunc = {
	["exact"] = {},
	["local"] = {},
	["whole"] = {},
	["alone"] = {"full_url_send_weibo","unknow_url_send_weibo","half_url_random_idx_send_weibo","half_url_incr_idx_send_weibo","half_url_mult_idx_send_weibo"},
}

ranklist = {-->other default is 0,the higher rank function will be run the first.
}

slotlist = {
}

OWN_INFO = {
	DEFAULT_APP_PATH = "./lua/appcenter/data/",

	LOGLV = 0,
	DEFAULT_LOG_PATH = "./logs/",
	OPEN_LOGS_CLASSIFY = false,


	key_len = 20,
	bit_step = 5, --> must <= 6
	--> 20 * 5 kinds
}
