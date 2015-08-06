local pool = require('pool')
local APP_POOL= require('pool')
local only = require('only')
local supex = require('supex')
local utils = require('utils')
local link = require('link')
local cjson = require('cjson')
local weibo_api = require('weibo')
--local redis_api = require('redis_short_api')
local redis_api = require('redis_pool_api')
local weibo_statistics = require('weibo_statistics')

module('p2p_over_speed', package.seeall)

local app_info = {
	appKey = "3406572696",
	secret = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
}

function handle()
	only.log('D', string.format("[p2p_over_speed]"))

	local lon       = pool["OUR_BODY_TABLE"]["longitude"] 
	and pool["OUR_BODY_TABLE"]["longitude"][1] or 0
	local lat       = pool["OUR_BODY_TABLE"]["latitude"] 
	and pool["OUR_BODY_TABLE"]["latitude"][1] or 0
	local speed     = pool["OUR_BODY_TABLE"]["speed"] 
	and pool["OUR_BODY_TABLE"]["speed"][1]  or 0
	local direction = pool["OUR_BODY_TABLE"]["direction"] 
	and pool["OUR_BODY_TABLE"]["direction"][1] or -1
	local altitude  = pool["OUR_BODY_TABLE"]["altitude"] 
	and pool["OUR_BODY_TABLE"]["altitude"][1] or 0
	local accountID = pool["OUR_BODY_TABLE"]["accountID"]
	if accountID == nil then
		return false;
	end
	local carry_key = accountID ..  ":"  .. "overSpeedCarry"
	local ok, ret = redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', carry_key )
	if (not ok) or (not ret) then
		return false
	end
	only.log('D', string.format("[ret = %s]", ret))

	local  flag, limit_speed =  string.match(ret, "([^:]+):([%w]+)")
	if flag == nil or limit_speed == nil then
		only.log('E', string.format("[flag == nil][limit_speed == nil]"))
		return false;
	end
	-- check for index, base_mileage, now_mileage
	only.log('D', string.format("[flag:%s][limit_speed:%s]", 
	flag,limit_speed))

	local text;
	local txt = {
		--[1] = "你当前的时速为%s,已超速",
		--[1] = "%s限速%s，您已超速",
		[1] = "当前道路限速%s，您已超速",
	}
	local txt_tmp = {
		[1] = "你超速了，还是慢点开吧。",
		[2] = "你一直超速，你家人知道不。",
		[3] = "再不慢点，我就要插播广告了。",
		[4] = "开车不超速，你好，我好，大家好。",
	}
	local index = 0
	if tonumber(flag) == 0 then
		text = string.format(txt[1], limit_speed)
		index = tonumber(limit_speed)
	else
		index = tonumber(os.time() % 9 + 1)
		text = txt_tmp[index]
	end
	--[[
	http://127.0.0.1/productList/overSpeed/20060.amr
	http://127.0.0.1/productList/overSpeed/20070.amr
	http://127.0.0.1/productList/overSpeed/20080.amr
	http://127.0.0.1/productList/overSpeed/20090.amr
	http://127.0.0.1/productList/overSpeed/20100.amr
	http://127.0.0.1/productList/overSpeed/20110.amr
	http://127.0.0.1/productList/overSpeed/20120.amr
	http://127.0.0.1/productList/overSpeed/20001.amr
	http://127.0.0.1/productList/overSpeed/20002.amr
	http://127.0.0.1/productList/overSpeed/20003.amr
	http://127.0.0.1/productList/overSpeed/20004.amr
	--]]
	local tb = {
		[60]    = 20060,
		[70]    = 20070,
		[80]    = 20080,
		[90]    = 20090,
		[100]   = 20100,
		[110]   = 20110,
		[120]   = 20120,
		[1]     = 20001,
		[2]     = 20002,
		[3]     = 20003,
		[4]     = 20004,
                [5]     = 20005,
                [6]     = 20006,
                [7]     = 20007,
                [8]     = 20008,
                [9]     = 20009,
	}
	local fileurl = nil
	local amr_host  = link["OWN_DIED"]["http"]["overSpeedAMR"]["host"]
	if tb[index] then
		fileurl = "http://" .. amr_host .. "/productList/overSpeed/" .. tb[index] .. ".amr"
	else
		--> gen amr file 
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
		if not ok or not resp then
			only.log('D',string.format("fileurl is nil"))
			fileurl = nil
		end
		if ok and resp then
			only.log('D', resp)
			local jo = utils.parse_api_result( resp, "spx_txt_to_voice")
			fileurl =  jo and jo["url"] or nil
		end
		if fileurl then
			only.log('D',string.format("fileurl:%s",fileurl))
		else
			return false
		end
	end

	local wb = {
		multimediaURL = fileurl,
		receiverAccountID = accountID,
		interval = 30,
		level = 30,
		content = text,
		senderType = 2,
		--sourceId = fileID,
	}
	if lon and lat then
		wb['senderLongitude'] = lon
		wb['senderLatitude'] = lat
	end
	wb["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
	wb["senderSpeed"] =  string.format('[%s]', speed and math.ceil(speed) or 0)
	wb["receiverSpeed"] =  string.format('[%s,900]', math.ceil(tonumber(limit_speed)*1.05))
	if altitude then
		wb["senderAltitude"] =  altitude
	end

	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok, ret = weibo_api.send_weibo( server, "personal", wb, app_info["appKey"], app_info["secret"] )

	--[[
	ok = true
	ret = {bizid = "a40ab9142085ce11e48d1a000c29bc68cf", count = 1}
	--]]

	--only.log('D', string.format("[ok = %s][ret = %s]", ok, ret))
	if ok then
		--only.log('D', scan.dump(ret))
                --[[
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
                ]]--
                weibo_statistics.Statistics(ret['bizid'],app_info["appKey"])
	else
		only.log('D', "send weibo error")
	end

end
