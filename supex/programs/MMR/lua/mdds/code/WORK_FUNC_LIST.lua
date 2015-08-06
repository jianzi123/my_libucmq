local http_api = require("http_short_api")
local APP_CONFIG_LIST = require('CONFIG_LIST')
local common_cfg = require("cfg")
local mysql_api = require("mysql_pool_api")
local redis_api = require("redis_pool_api")
local cjson = require("cjson")
local APP_POOL = require("pool")
local only = require('only')
local link = require("link")
local utils = require("utils")
local supex = require("supex")
local judge = require("judge")
local link = require("link")
local weibo = require('weibo')


module("WORK_FUNC_LIST", package.seeall)

-->> public
OWN_JOIN = {
	cause ={
		trigger_type = "every_time,one_day,power_on,fixed_time",
	}
}
OWN_MUST = {
	cause = {
		trigger_type = "every_time",
		fix_num = 1,
		delay = 0
	},
}

-->> private
OWN_HINT = {
	-->> exact local whole
	app_task_forward = {},
	-->> alone
	full_url_send_weibo = {},--{fullURL = "[nickname], [content]"},

	half_url_mult_idx_send_weibo = {
		idx_key = "4MilesAheadPositionTypeSet",
		halfURL = 'http://127.0.0.1/productList/POIRemind/%s.amr'
	},
}

OWN_ARGS = {
	-->> exact local whole
	app_task_forward = {
		app_uri = "a_d_xxxxxxxxxxx",
	},

	-->> alone
	full_url_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		fullURL = 'http://127.0.0.1/productList/xxxxx/xxxx.amr',
		carry_key = false,
	},

	half_url_random_idx_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		idx_base = "30000",
		idx_max = 1,
		halfURL = 'http://127.0.0.1/productList/xxxxx/%s.amr',
		carry_key = false,
	},
	half_url_mult_idx_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		idx_key = "4MilesAheadPositionTypeSet",
		halfURL = 'http://127.0.0.1/productList/xxxxx/%s.amr',
		carry_key = "xxx",
	},
}

function app_task_forward( app_name )
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local ok = judge.freq_filter(app_name, accountID)
	if not ok then
		only.log("D", "freq_filter false")
		return
	end

	local app_uri = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["app_task_forward"]["app_uri"]
	local path = string.gsub(app_uri, "?.*", "")
	local app_srv = link["OWN_DIED"]["http"][ path ]
	local data = utils.compose_http_json_request(app_srv, app_uri, nil, APP_POOL["OUR_BODY_DATA"])
	http_api.http(app_srv, data, false)
end
