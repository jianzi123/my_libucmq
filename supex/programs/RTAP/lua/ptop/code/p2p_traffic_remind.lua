local pool          = require('pool')
local APP_POOL      = require('pool')
local only          = require('only')
local supex         = require('supex')
local utils         = require('utils')
local link          = require('link')
local cjson         = require('cjson')
local weibo_api     = require('weibo')
local scan          = require('scan')
--local redis_api = require('redis_short_api')
local redis_api = require('redis_pool_api')
local cutils		= require('cutils')

module('p2p_traffic_remind', package.seeall)

local app_info = {
	appKey = "1854678079",
	secret = "439581000FF13D23F4A23D7DD61C2144B10AFA64",
}


local function check_is_ok (tb, roadInfo)
	if roadInfo['averageSpeed'] and
		tonumber(roadInfo['averageSpeed']) <= 40 then
		return true
	end
	if roadInfo['averageSpeed'] and
		tonumber(roadInfo['averageSpeed']) > 40 
		and roadInfo['roadLength'] then
		tb['overSpeedLen'] = tb['overSpeedLen'] and tonumber(tb['overSpeedLen'])  or 0
		local roadLen = tonumber(roadInfo['roadLength'])
		if tb['overSpeedLen'] +  roadLen < 100 then
			return true
		end
	end
	return false
end

local function add (tb, roadInfo) 
	if not tb['speed'] then
		tb['speed'] = {}
	end
	if not tb['lowSpeedLen'] then
		tb['lowSpeedLen'] = 0
		tb['lon'] = roadInfo['L1']
		tb['lat'] = roadInfo['B1']
	end
	if not tb['overSpeedLen'] then
		tb['overSpeedLen'] = 0
	end
	if not tb['maxSpeed'] then
		tb['maxSpeed'] = 0
	end
	table.insert(tb['speed'], roadInfo['averageSpeed'])
	local speed = tonumber(roadInfo['averageSpeed'])
	if speed > 40 then
		tb['overSpeedLen'] = tb['overSpeedLen'] + roadInfo['roadLength']
	else
		tb['lowSpeedLen'] = tb['lowSpeedLen'] + roadInfo['roadLength']
	end
	if speed > tb['maxSpeed'] then
		tb['maxSpeed'] = speed
	end
end

local function get_lowLen_txt(lowLen) 
	local txt = '' 
	if lowLen < 1000 then 
		txt =  math.floor(lowLen/10) * 10   
		txt = txt .. '米' 
	else 
		local a = math.floor(lowLen / 1000)
		local b = math.floor((lowLen  - a * 1000) / 100) 
		txt = a .. '点' .. b .. '公里'
	end  
	return txt  
end


local function get_mapabc_traffic(accountID, lon, lat, dir, speed )
	local imei = nil
	if string.len(accountID) ~= 15 then
		ok, imei =  redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:IMEI", accountID))
	else
		imei = accountID
	end

	local wb = {
		appKey = app_info['appKey'],
		imei        = imei,
		longitude   = lon,
		latitude    = lat,
		direction   = dir,
	}
	local sign = utils.gen_sign(wb, app_info['secret'])
	wb['sign'] = sign

	local serv = link["OWN_DIED"]["http"]["/roadRankapi/v1/getFrontSpeedByLBS"]
	local req = utils.compose_http_kvps_request(serv, "/roadRankapi/v1/getFrontSpeedByLBS", nil, wb, "POST")

	only.log('D', string.format("req:%s", req))
	local ok, ret = supex.http(serv["host"], serv["port"], req, #req)
	only.log('D', string.format("ret:%s", ret))
	if not ok or not ret then return false end
	ret = string.match(ret, '{.+}')
	--[[
	ret =  [[
	{"ERRORCODE":"0", "RESULT":[{"vehicleCount":1,"trust":"0.806","roadLevel":"0","L2":"121.388407","roadID":"76125250","B2":"31.220070","roadName":"长宁路","direction":"110","B1":"31.219955","L1":"121.385398","speedLimit":"50","averageSpeed":"59","time":"1407210985","maxSpeed":"59","roadLength":"286"},{"vehicleCount":1,"trust":"0.26","roadLevel":"0","L2":"121.406018","roadID":"76142353","B2":"31.218956","roadName":"长宁路","direction":"110","B1":"31.218797","L1":"121.404216","speedLimit":"50","averageSpeed":"63","time":"1407210985","maxSpeed":"63","roadLength":"172"},{"vehicleCount":1,"trust":"0.74","roadLevel":"0","L2":"121.406018","roadID":"76106781","B2":"31.218956","roadName":"长宁路","direction":"110","B1":"31.218797","L1":"121.404216","speedLimit":"50","averageSpeed":"40","time":"1407210985","maxSpeed":"40","roadLength":"172"},{"vehicleCount":1,"trust":"0.48","roadLevel":"0","L2":"121.373097","roadID":"76125927","B2":"31.221511","roadName":"北翟路","direction":"110","B1":"31.221533","L1":"121.372320","speedLimit":"50","averageSpeed":"51","time":"1407210985","maxSpeed":"51","roadLength":"74"}]}
	]]
	--]]
	local ok, res = pcall(cjson.decode, ret) 
	if not ok then 
		only.log('W', "\r\nfailed to decode result")
	end  
	only.log('D', string.format("[res:%s]", scan.dump(res)))
	if(tonumber(res['ERRORCODE']) ~= 0) then
		only.log('W', "get traffic Information failed")
		return false
	end
	local traffic =  res['RESULT'] 
	if not traffic then
		only.log('W', "no traffic")
		return false
	end

	--[[
	only.log('D',string.format('[lon:%s][lat:%s]', lon + 0.02, lat + 0.02))
	local dis = cutils.gps_distance(lon, lat, lon + 0.02,  lat + 0.02)
	dis = dis and tonumber(dis) or 0
	only.log('D', string.format('[dis:%s]', dis))
	dis = math.floor(dis)
	only.log('D', string.format('[dis:%s]', dis))
	--]]

	local j = 0
	for i = 1, #traffic do
		local roadInfo = traffic[i]
		local avgSpeed = roadInfo['averageSpeed']
		if avgSpeed and
			tonumber(avgSpeed) <= 40 then
			j = i
			break
		end
	end
	only.log('D', string.format('[j = %d]', j))
	if j == 0 then
		return nil
	end
	local dis = cutils.gps_distance(lon, lat, traffic[j]['L1'], traffic[j]['B1'])
	dis = dis and tonumber(dis) or 0

	local cast = {}
	local tb = {}
	for i = j, #traffic do
		if check_is_ok(tb, traffic[i]) then
			add(tb, traffic[i])
		else
			table.insert(cast, tb)
			tb = {}
		end
	end

	if next(tb) then
		table.insert(cast, tb)
	end

	if #cast < 1 then
		return  false
	end

	local txt = ''
	if dis < 500 then
		txt = '正在经过,'
	else
		txt = '前方经过,'
	end

	for i = 1, #cast do
		local tb = cast[i]
		local maxSpeed = tb['maxSpeed']
		local lowLen = tb['lowSpeedLen'] + tb['overSpeedLen'] 
		lowLen = get_lowLen_txt(lowLen)
		local avgSpeed = 0
		for j = 1, #tb['speed'] do
			avgSpeed = avgSpeed + tonumber(tb['speed'][i])
		end
		avgSpeed = math.floor(avgSpeed / (#tb['speed']))
		if avgSpeed < 20 then
			txt = txt .. string.format('拥堵路段,拥堵距离%s,平均速度%s,最大速度%s,', 
			lowLen, avgSpeed,maxSpeed)
		else
			txt = txt .. string.format('缓行路段,缓行距离%s,平均速度%s,最大速度%s,', 
			lowLen, avgSpeed,maxSpeed)
		end
	end

	-- 正在经过拥堵（缓行）路段，拥堵（缓行）距离***米，平均速度***。最大速度****；



	--[[
	--前方4公里，***米（十位数取整）拥堵，***米（十位数取整）缓行，，平均速度***，最高速度***；
	local txt_000 = "前方4公里"
	local txt_001 = "%s米拥堵"				--[平均时速低于20-40km/h的路段公里数]
	local txt_002 = "%s米缓行"				-- [平均时速低于20km/h的路段公里数]
	local txt_003 = "平均速度%s，最高速度%s"
	--local txt_001 = "前方%s米平均时速%s，最大通行时速%s"
	--local txt_002 = "其中有%s米缓行路段" --[平均时速低于20-40km/h的路段公里数]
	--local txt_003 = "有%s米拥堵路段"     -- [平均时速低于20km/h的路段公里数]
	local avg_speed = 0
	local max_speed = 0
	local cnt = #traffic
	if cnt == 0 then
		only.log('W', "[taffice][taffic info is nil]")
		return false
	end
	local slow_length = 0
	local block_length = 0
	local max_meters = 0
	for i, v in pairs(traffic) do
		avg_speed = v['averageSpeed'] + avg_speed
		max_speed = v['maxSpeed'] + max_speed
		if tonumber(v['averageSpeed']) < 20  then
			block_length = block_length + v['roadLength']
		end
		if tonumber(v['averageSpeed']) >= 20 and tonumber(v['averageSpeed']) <= 40  then
			slow_length = slow_length + v['roadLength']
		end
		max_meters = max_meters + v['roadLength']
	end
	if max_meters < 200 then
		only.log('W', string.format("[max_meters:%s][less than 200 meters]", max_meters))
		return false
	end
	avg_speed = math.ceil(avg_speed / cnt )
	max_speed = math.ceil(max_speed / cnt )
	max_meters = math.ceil(max_meters / 10 ) * 10
	local txt = string.format(txt_000)
	if block_length ~= 0 then
		txt = txt ..',' .. string.format(txt_001, math.floor(block_length/10) * 10)
	end
	if slow_length  ~= 0 then
		txt = txt ..',' .. string.format(txt_002, math.floor(slow_length/10) * 10)
	end
	txt = txt ..'，' .. string.format(txt_003, avg_speed, max_speed)
	--]]

	return txt
	--return traffic
end


local function get_fileurl( text )
	local wb = {
		appKey = app_info['appKey'],
		text = text,
	}
	local sign = utils.gen_sign(wb, app_info['secret'])
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
	local gps_data = pool["OUR_BODY_TABLE"]

	local current_longitude = gps_data["longitude"][1]
	local current_latitude = gps_data["latitude"][1]
	local accountID = gps_data["accountID"]
	local altitude  = pool["OUR_BODY_TABLE"]["altitude"][1]
	local valid_direction = nil
	local valid_speed = nil

	--> get a valid package, -1 is bad
	for k,v in ipairs(gps_data["direction"] or {}) do
		if v ~= -1 then
			valid_direction = v
			valid_speed = gps_data["speed"][ k ]
			break
		end
	end

	if valid_direction and valid_direction ~= -1 then
		-->> traffic
		local text = get_mapabc_traffic(accountID, current_longitude, current_latitude, valid_direction, valid_speed )
		if text then
			--local content = text .. ",来自高德路况。"
			local content = text 
			local fileurl = get_fileurl( content )
			only.log('D', fileurl)
			--> send weibo
			local wb = {
				multimediaURL = fileurl,
				receiverAccountID = accountID,
				interval = 40,
				level = 30,
				content = content,
				senderType = 2,
			}
			wb['senderLongitude'] = current_longitude 
			wb['senderLatitude'] = current_latitude 
			wb["senderDirection"] = string.format('[%s]', valid_direction and math.ceil(valid_direction) or -1)
			wb["senderSpeed"] =  string.format('[%s]', valid_speed and math.ceil(valid_speed) or 0)
			if altitude then
				wb["senderAltitude"] =  altitude
			end
			local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
			local ok, ret = weibo_api.send_weibo( server, "personal", wb, app_info['appKey'], app_info['secret'])
			if ok then
				--[[
				--]]
				local bizid = ret['bizid']
				local time = os.time()
				local travelID  = nil
				local appKey = app_info['appKey']
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
end
