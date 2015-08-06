local pool = require('pool')
local APP_POOL = require('pool')
local only = require('only')
local supex = require('supex')
local utils = require('utils')
local link = require('link')
local cjson = require('cjson')
local weibo_api = require('weibo')
--local redis_api = require('redis_short_api')
local redis_api = require('redis_pool_api')

module('p2p_get_txt2voice', package.seeall)


app_info = {
	appKey = "2491067261",
	secret = "52E8DCDEB8DBBAD220652851AE34339B008F5B48",
}

local function get_fileurl( text )
	local wb = {
		appKey = app_info['appKey'],
		text = text,
	}
	local secret = app_info['secret']
	local sign = utils.gen_sign(wb, secret)
	wb['sign'] = sign

	local serv = link["OWN_DIED"]["http"]["dfsapi/v2/txt2voice"]
	--local serv = link["OWN_DIED"]["http"]["spx_txt_to_voice"]
	--local req = utils.compose_http_kvps_request(serv, "spx_txt_to_voice", nil, wb, "POST")
	local req = utils.compose_http_kvps_request(serv, "dfsapi/v2/txt2voice", nil, wb, "POST")
	only.log('D', req)

	local ok, resp = supex.http(serv["host"], serv["port"], req, #req)
	if not ok or not resp then return nil end
	only.log('D', resp)

	local jo = utils.parse_api_result( resp, "spx_txt_to_voice")
	return jo and jo["url"] or nil
end


function handle()
	--30,35,40,45,50,55,60,70,80,90,100,110,120
	local tb = {
		--[0] = '我们是中国人，90号汽油:6.94元，93号汽油:6.23元，97号汽油:6.59元'
		[0] = '90号汽油:6点九四元，93号汽油:6点二三元，97号汽油:6点五九元'
	}
	--local poiType = 1123110

	for i, v in pairs(tb) do
		local fileurl = get_fileurl(v)
		--only.log('E', string.format("[%s] = '%s', --%s",poiType .. i, fileurl, v))
		only.log('E', string.format("[fileurl=%s]", fileurl))
	end
end
