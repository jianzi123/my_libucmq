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
local weibo_statistics = require('weibo_statistics')

module('p2p_weather_forcast', package.seeall)

local app_info = {
	-- over speed
	--[[
	appKey = "3406572696",
	secret = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
	--]]
	-- weather forcast 
	appKey = "2496286142",
	secret = "38CFE44CB46050D4FE30CCF22C213C9CD0442614",
}

function handle()
	only.log('D', string.format("[p2p_weather_forcast]"))

	local speed     = pool["OUR_BODY_TABLE"]["speed"] and pool["OUR_BODY_TABLE"]["speed"][1]
	local direction = pool["OUR_BODY_TABLE"]["direction"] and pool["OUR_BODY_TABLE"]["direction"][1]
	local altitude  = pool["OUR_BODY_TABLE"]["altitude"] and pool["OUR_BODY_TABLE"]["altitude"][1]
	local accountID = pool["OUR_BODY_TABLE"]["accountID"]
	if accountID == nil then
		return false;
	end
    local ok,current = redis_api.cmd('private', pool["OUR_BODY_TABLE"]["accountID"], 'get',accountID .. ':currentBL')
    if not ok then
        only.log('E',"get currentBL is fail")
    end
    local current_table = utils.str_split(current,",")
    local lon = current_table[1]
    local lat = current_table[2]
    if not lon or not lat then
        only.log('E',"grid is nil")
        return false
    end
	local grid = string.format('%d&%d', tonumber(lon) * 100, tonumber(lat) * 100)
	only.log("D", "redis grid key:" .. grid)

	--[[
	local ok, code_jo = redis_api.cmd('mapGridOnePercent',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', grid)
	if (not ok) or (not code_jo) then
		only.log("W", "can't get code from redis!")
		return false
	end
	only.log("D", "json data:" .. code_jo)
	local ok, code_tab = pcall(cjson.decode, code_jo)
	if not ok then
		only.log("E", "can't decode code! " .. code_tab)
		return false
	end
	--]]

	local ok, code_jo = redis_api.cmd('mapGridOnePercent01',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hgetall', grid)
	if (not ok) or (not code_jo) then
		only.log("W", "can't get code from redis!")
		return false
	end
	only.log('D', string.format("[code_jo = %s]", scan.dump(code_jo)))

	local provinceCode = code_jo['provinceCode']
	local cityCode = code_jo['cityCode']

	if not cityCode and not provinceCode then
		only.log("E", "can't get cityCode from redis!")
		return false
	end

	if code_jo['provinceName'] == '上海市' or 
		code_jo['provinceName'] == '北京市' or 
		code_jo['provinceName'] == '重庆市' or 
		code_jo['provinceName'] == '天津市' then
		cityCode = code_jo['provinceCode']
	end

	local key = cityCode .. ":weatherForecast"
	only.log('D', string.format("[key:%s]", key))
	local ok, ret = redis_api.cmd("public",APP_POOL["OUR_BODY_TABLE"]["accountID"], "hgetall", key)
	if (not ok) or (not ret) then
		only.log('D', accountID .. ':cityCode is NULL')
	end
	only.log('D', string.format('[ret:%s]', ret))
	for i, v in pairs(ret or {}) do
		only.log('D', string.format('[i:%s][v:%s]', 
		i, v))
	end

	only.log('D', string.format("[ret[text]:%s]", ret['text']))
	only.log('D', string.format("[ret[cityName]:%s]", ret['cityName']))
	only.log('D', string.format("[ret[temperature]:%s]", ret['temperature']))
	--[[
	[weather_forecast_key:110000:weatherForecastSex][field:cityName][value:北京]
	[weather_forecast_key:110000:weatherForecastSex][field:lastUpdate][value:2014-08-20 21:35:53]
	[weather_forecast_key:110000:weatherForecastSex][field:cityCode][value:110000]
	[weather_forecast_key:110000:weatherForecastSex][field:temperature][value:27]
	[weather_forecast_key:110000:weatherForecastSex][field:text][value:多云]
	--]]
	if not ret['text'] or not ret['cityName'] or not ret['temperature'] then
		return false
	end

	local cityName      = ret['cityName'] or "北京"
	local state         = ret['text'] or "大雨"
	local temperature   = ret['temperature'] or "28"
	local lastUpdate    = ret['lastUpdate'] or '2014-08-21 00:12:23'
	local today         = os.date("%Y-%m-%d")

	if lastUpdate < today then
		only.log('D', string.format("[當前的天氣信息不是today的]"))
		return false
	end

	local text;
	local txt = {
		[1] = "%s当前天气%s，温度%s度",
	}
	text = string.format(txt[1], cityName, state, temperature)
	-- http://172.16.21.177:3011/getCityWeatherAlert?provinceCode=120000&cityCode=120001
	
	local wb = {
		provinceCode = provinceCode,
		cityCode = code_jo['cityCode'],
	}
	local serv = link["OWN_DIED"]["http"]["/getCityWeatherAlert"]
	local req = utils.compose_http_kvps_request(serv, "getCityWeatherAlert", nil, wb, "GET")
	only.log('D', string.format("req:%s", req))
	local ok, ret = supex.http(serv["host"], serv["port"], req, #req)
	only.log('D', string.format("[ok:%s][ret:%s]", ok, ret))
	
	--[[
	ok  = true
	ret = [[{"error":0,"result":{"title":["上海市气象台发布大雾黄色预警"],
	"link":["http://www.weather.com.cn/alarm/newalarmcontent.shtml?file=10102-20150105064500-1202.html"],
	"atype":["大雾"],"alevel":["黄色"],"astatus":["预警中"],
	"img":["http://www.weather.com.cn/m2/i/alarm_s/1202.gif"],
	"description":["预计今天上午以前本市大部地区有能见度小于500米的大雾，请注意防范。"],
	"pubDate":["2015-01-05 06:45:00"],
	"province":"上海市","city":"","alertNum":"201501050919583671大雾黄色",
	"cityCode":"310000","desc":"能见度不好，千万不要开快车噢。"}}]]
	--]]

	if not ok or not ret then return false end
	ret = string.match(ret, '{.+}')
	local ok, res = pcall(cjson.decode, ret) 
	if not ok then 
		only.log('W', "\r\nfailed to decode result")
	end  
	only.log('D', string.format("res:%s", scan.dump(res)))

	if res['error'] == 0 and res['result']['title']  and res['result']['desc'] then
		text = text .. '，' .. res['result']['title'] .. '，' .. res['result']['desc']
	end

	--> send weibo
	local wb = {
		appKey = app_info["appKey"],
		text = text,
	}
	local secret = app_info["secret"]
	local sign = utils.gen_sign(wb, secret)
	wb['sign'] = sign

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
	if altitude then
		wb["senderAltitude"] =  altitude
	end
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok, ret = weibo_api.send_weibo( server, "personal", wb, app_info["appKey"], app_info["secret"] )
	if ok then
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
		--]]
                weibo_statistics.Statistics(ret['bizid'],app_info["appKey"])
	end

end
