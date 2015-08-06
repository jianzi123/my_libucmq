local pool = require('pool')
local APP_POOL  = require('pool')
local only = require('only')
local supex = require('supex')
local utils = require('utils')
local link = require('link')
local cjson = require('cjson')
local weibo_api = require('weibo')
--local redis_api = require('redis_short_api')
local redis_api = require('redis_pool_api')

module('p2p_continuous_driving_mileage_remind', package.seeall)

local app_info = {
	appKey = "1106086205",
	secret = "C6BF9297AAE1A12E267FBE4DED1C99FB54132607",
}

function handle()
	only.log('D', string.format("[p2p_continuous_driving_mileage_remind]"))

	local lon       = pool["OUR_BODY_TABLE"]["longitude"] and pool["OUR_BODY_TABLE"]["longitude"][1]
	local lat       = pool["OUR_BODY_TABLE"]["latitude"] and pool["OUR_BODY_TABLE"]["latitude"][1]
	local speed     = pool["OUR_BODY_TABLE"]["speed"] and pool["OUR_BODY_TABLE"]["speed"][1]
	local direction = pool["OUR_BODY_TABLE"]["direction"] and pool["OUR_BODY_TABLE"]["direction"][1]
	local altitude  = pool["OUR_BODY_TABLE"]["altitude"] and pool["OUR_BODY_TABLE"]["altitude"][1]
	local accountID = pool["OUR_BODY_TABLE"]["accountID"]
	if accountID == nil then
		return false
	end
    local ok,nickname = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',accountID .. ':nickname')
    if not ok or not nickname then
        nickname = '道客'
    end
	local carry_key = accountID ..":continuousDrivingCarryData"
	local ok, ret = redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', carry_key )
	only.log('D', string.format("[carry data  = %s]", ret))

	local data_type, useless_argument, actual_mileage , max_speed, avg_speed, stop_time= string.match(ret, "([%.%w]+):([%.%w]+):([%.%w]+):([%.%w]+):([%.%w]+):([%.%w]+)")
	if data_type == nil or actual_mileage ==nil or max_speed ==nil or avg_speed ==nil or stop_time == nil then
		only.log('E', string.format("[data_type == nil  == nil or actual_mileage == nil or max_speed ==nil or avg_speed ==nil or stop_time == nil]"))
		return false
	end

	only.log('D',string.format("[data_type:%s]", data_type))
	only.log('D',string.format('[actual_mileage:%s]',actual_mileage))
    only.log('D',string.format('[max_speed:%s]',max_speed))
    only.log('D',string.format('[avg_speed:%s]',avg_speed))
	only.log('D',string.format('[stop_time:%s]',stop_time))
	data_type = tonumber(data_type) or 0
	actual_mileage = tonumber(actual_mileage) or 0
    max_speed = tonumber(max_speed) or 0
    avg_speed = tonumber(avg_speed) or 0
    stop_time = tonumber(stop_time) or 0
	if not (data_type == 2)  then
                only.log('D', "data_type error ,data_type is :" .. tostring(data_type))
		return false
	end
	local text;
    if math.modf(stop_time / 3600) > 0 then
        local stop_time_hour = math.modf(stop_time / 3600)
        local stop_time_minute = math.modf((stop_time - stop_time_hour * 3600) / 60)
        local stop_time_second = stop_time % 3600 % 60
        if stop_time_minute == 0 or stop_time_second == 0 then 
            return false
        end
	    local txt = {
		    [1] = "亲爱的%s, 你已经驾驶了%s公里, 最高时速%s, 平均时速%s, 停止时长%s小时%s分钟%s秒",
		    [2] = "亲爱的%s, 你已经驾驶了%s公里, 最高时速%s, 平均时速%s, 停止时长%s小时%s分钟%s秒, 貌似你开快了",
		    [3] = "亲爱的%s, 你已经驾驶了%s公里, 最高时速%s, 平均时速%s，停止时间长%s小时%s分钟%s秒，开这么久休息一下吧",
        }

	    if max_speed < 125 and actual_mileage ~= 100 and actual_mileage ~= 200 and actual_mileage ~= 300 then
		    text = string.format(txt[1], nickname, actual_mileage, max_speed, avg_speed, stop_time_hour, stop_time_minute, stop_time_second)
	    elseif max_speed > 125 and actual_mileage ~= 100 and actual_mileage ~= 200 and actual_mileage ~= 300 then
		    text = string.format(txt[2], nickname, actual_mileage, max_speed, avg_speed, stop_time_hour, stop_time_minute, stop_time_second)
	    elseif actual_mileage == 100 or actual_mileage == 200 or actual_mileage == 300 then 
		    text = string.format(txt[3], nickname, actual_mileage, max_speed, avg_speed, stop_time_hour, stop_time_minute, stop_time_second)
	    else 
		    return false
	    end
            only.log('D', "text:" .. text)
        
    elseif math.modf(stop_time /60) > 0 then
        local stop_time_minute = math.modf(stop_time / 60)
        local stop_time_second = stop_time % 60
        if stop_time_second == 0 then
            return false 
        end
	    local txt = {
		    [1] = "亲爱的%s, 你已经驾驶了%s公里, 最高时速%s, 平均时速%s, 停止时长%s分钟%s秒",
		    [2] = "亲爱的%s, 你已经驾驶了%s公里, 最高时速%s, 平均时速%s, 停止时长%s分钟%s秒, 貌似你开快了",
		    [3] = "亲爱的%s, 你已经驾驶了%s公里, 最高时速%s, 平均时速%s，停止时长%s分钟%s秒，开这么久休息一下吧",
        }

	    if max_speed < 125 and actual_mileage ~= 100 and actual_mileage ~= 200 and actual_mileage ~= 300 then
		    text = string.format(txt[1], nickname, actual_mileage, max_speed, avg_speed, stop_time_minute, stop_time_second)
	    elseif max_speed > 125 and  actual_mileage ~= 100 and actual_mileage ~= 200 and actual_mileage ~= 300 then
		    text = string.format(txt[2], nickname, actual_mileage, max_speed, avg_speed, stop_time_minute, stop_time_second)
	    elseif actual_mileage == 100 or actual_mileage == 200 or actual_mileage == 300 then 
		    text = string.format(txt[3], nickname, actual_mileage, max_speed, avg_speed, stop_time_minute, stop_time_second)
	    else 
		    return false
	    end
            only.log('D', "text:" .. text)

    else
        local stop_time_second = stop_time
	    local txt = {
		    [1] = "亲爱的%s, 你已经驾驶了%s公里, 最高时速%s, 平均时速%s, 停止时长%s秒",
		    [2] = "亲爱的%s, 你已经驾驶了%s公里, 最高时速%s, 平均时速%s, 停止时长%s秒, 貌似你开快了",
		    [3] = "亲爱的%s, 你已经驾驶了%s公里, 最高时速%s, 平均时速%s，停止时长%s秒，开这么久休息一下吧",
        }

	    if  max_speed < 125 and actual_mileage ~= 100 and actual_mileage ~= 200 and actual_mileage ~= 300 then
		    text = string.format(txt[1], nickname, actual_mileage, max_speed, avg_speed, stop_time_second)
	    elseif  max_speed > 125 and actual_mileage ~= 100 and actual_mileage ~= 200 and actual_mileage ~= 300 then
		    text = string.format(txt[2], nickname, actual_mileage, max_speed, avg_speed, stop_time_second)
	    elseif actual_mileage == 100 or actual_mileage == 200 or actual_mileage == 300 then 
		    text = string.format(txt[3], nickname, actual_mileage, max_speed, avg_speed, stop_time_second)
	    else 
		    return false
	    end
            only.log('D', "text:" .. text)
    end
	--> send weibo
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
	local fileurl
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
	local wb = {
		multimediaURL = fileurl,
		receiverAccountID = accountID,
		interval = 300,
		level = 80,
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
	if altitude then
		wb["senderAltitude"] =  altitude
	end
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok, ret = weibo_api.send_weibo( server, "personal", wb, app_info["appKey"], app_info["secret"] )
	if ok then
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
