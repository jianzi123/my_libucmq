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

module('p2p_limit_speed_remind', package.seeall)

local app_info = {
	-- right but, can't use now
	appKey = "864537339",
	secret = "67E24F945F16F844F239B51C84643C5B2ADE125A",
	-- just for test
	--appKey = "699771737",
	--secret = "9296ED2B9D139D4730484AE4E01C6F17AE2685EA",
}

local function get_fileurl( text )
	local wb = {
		appKey = app_info["appKey"],
		text = text,
	}
	local secret = app_info["secret"]
	local sign = utils.gen_sign(wb, secret)
	wb['sign'] = sign

	--local serv = link["OWN_DIED"]["http"]["dfsapi/v2/txt2voice"]
	local serv = link["OWN_DIED"]["http"]["spx_txt_to_voice"]
	local req = utils.compose_http_kvps_request(serv, "spx_txt_to_voice", nil, wb, "POST")
	only.log('D', req)

	local ok, resp = supex.http(serv["host"], serv["port"], req, #req)
	if not ok or not resp then return nil end
	only.log('D', resp)

	local jo = utils.parse_api_result( resp, "spx_txt_to_voice")
	return jo and jo["url"] or nil
end

function handle()
	-->-->-->-->-->-->-->-->-->-->
	local gps_data = pool["OUR_BODY_TABLE"]

	local current_longitude = gps_data["longitude"][1]
	local current_latitude = gps_data["latitude"][1]
	local valid_direction = nil
	local valid_speed = nil
	local accountID = gps_data["accountID"]
	local idx_key = "isLimitSpeedRemind"

	--> get a valid package, -1 is bad
	for k,v in ipairs(gps_data["direction"] or {}) do
		if v ~= -1 then
			valid_direction = v
			valid_speed = gps_data["speed"][ k ]
			break
		end
	end

	if valid_direction and valid_direction ~= -1 then
		-->> limit_speed
		local ok, ret = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':' .. idx_key)
		if (not ok) or (not ret) then
			return false
		end
		only.log('D', string.format("[ret = %s]", ret))
		local a = string.find(ret, ":")
		local road_name = string.sub(ret, 1, a - 1)
		local limit_speed  = string.sub(ret, a + 1)
		if road_name == nil or limit_speed == nil then
			only.log('E', string.format("[road_name == nil or limit_speed == nil]"))
			return false;
		end
		local content   = string.format("您当前的行驶在%s，限速%s公里每小时，请注意车速安全驾驶", 
		road_name, limit_speed)
		local fileurl   = get_fileurl( content )
		if fileurl then
			only.log('D', fileurl)
		else
			return false
		end
		--> send weibo
		local wb = {
			multimediaURL = fileurl,
			receiverAccountID = gps_data["accountID"],
			interval = 30,
			level = 70,
			content = content,
			senderType = 2,
		}

		local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
		local ok, ret = weibo_api.send_weibo( server, "personal", wb, app_info["appKey"], app_info["secret"] )
		if ok then
			--only.log('D', scan.dump(ret))
			local bizid = ret['bizid']
			local time = os.time()
			local travelID  = nil
			local appKey = app_info["appKey"]
			local ok, imei =  redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:IMEI", accountID))
			if ok and imei then
				ok, travelID = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:travelID", imei))
			end
			local ok_date, cur_date = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:speedDistribution", accountID))
			if not ok_date or not cur_date then
				cur_date = os.date("%Y%m")
				redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'set', string.format("%s:speedDistribution", accountID), cur_date)
			end 
			if ok and imei and accountID and travelID and cur_date then
				local datacore_statistics_var_key = accountID .. ":" .. travelID .. ":" .. appKey ..":" .. cur_date
				local datacore_statistics_var_value = bizid .. ":" .. time
				redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'sadd', datacore_statistics_var_key, datacore_statistics_var_value)
				redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'expire', datacore_statistics_var_key, 48*3600)

			end
		end
	end
end
