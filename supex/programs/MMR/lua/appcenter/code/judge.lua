-- auth: baoxue
-- time: 2014.04.27

local redis_api = require('redis_pool_api')
local APP_CONFIG_LIST = require('CONFIG_LIST')
local APP_POOL = require('pool')
local cjson = require('cjson')
local link  = require('link')
local cutils = require('cutils')
local supex = require('supex')
local utils = require('utils')
local only = require('only')
local http_short = require('http_short_api')
local weibo              = require("weibo")
local init_data = require("init_data")


module('judge', package.seeall)


function freq_filter( app_name, uid )
	local cause = APP_CONFIG_LIST["OWN_LIST"][app_name]["ways"]["cause"]
	local class = cause["trigger_type"]
	local num = cause["fix_num"]
	local delay = cause["delay"]
	--> func list
	local check_list = {
		every_time = function( ... )
			return true
		end,
		once_life = function( ... )
			local keyct = string.format("%s:onceAllLife", uid)
			local ok,val = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sismember", keyct, app_name)
			if not ok then return false end
			if not val then
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sadd", keyct, app_name)
				return true
			else
				return false
			end
		end,
		power_on = function( ... )
			local keyct = string.format("%s:oncePowerOn", uid)
			local ok,val = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sismember", keyct, app_name)
			if not ok then return false end
			if not val then
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sadd", keyct, app_name)
				return true
			else
				return false
			end
		end,
		one_day = function( ... )
			local keyct = string.format("%s:%s:everyDay", uid, app_name)
			local ok,val = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", keyct)
			if not ok then return false end
			if not val then
				local over = 86400 - ((os.time() + 28800)%(86400))
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "set", keyct, 1)
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "expire", keyct, over)
			else
				if tonumber(val) >= num then return false end
				--[[
				if delay > 0 then
				local keydy = string.format("%s:%s:everyDelay", uid, app_name)
				local ok,val = redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", keydy)
				if (not ok) or (not val) then return false end
				redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "set", keydy, 1)
				redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "expire", keydy, delay)
				end
				]]--
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "incr", keyct)
			end
			return true
		end,
		fixed_time = function( ... )
			local keyct = string.format("%s:%s:fixedInterval", uid, app_name)
			local ok,val = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", keyct)
			if not ok then return false end
			if not val then
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "set", keyct, 1)
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "expire", keyct, delay)
			else
				if tonumber(val) >= num then return false end
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "incr", keyct)
			end
			return true
		end,
	}
	return check_list[ class ]( )
end

local function get_road_info(lon, lat, back_type, dir)
	local wb = {
		appKey = "3406572696",
		longitude = lon,
		latitude = lat,
		altitude = "0",
		returnSign = back_type,
		direction = dir or "-1",
		time = "070420",
		speed = "0",
	}
	local sign = utils.gen_sign(wb, "04A069DD5AAFE20712CFE846650E02D239C1D4C1")
	wb['sign'] = sign

	local serv = link["OWN_DIED"]["api"]["point_match_road"]
	local body = utils.gen_url(wb)
	local data = utils.post_data("mapapi/v2/pointMatchRoad", serv, body)
	only.log('D', data)
	local ok, ret = supex.http(serv['host'], serv['port'], data, #data)
	only.log('D', ret)
	ret = string.match(ret, '{.+}')
	only.log('D', string.format("[get_road_info_ret:%s]", ret))
	local ok, res = pcall(cjson.decode, ret)
	if not ok then
		only.log('D', "\r\nfailed to decode result")
	end
	return res
end

function freq_regain( app_name, uid )
	local cause = APP_CONFIG_LIST["OWN_LIST"][app_name]["ways"]["cause"]
	local class = cause["trigger_type"]
	local num = cause["fix_num"]
	local delay = cause["delay"]
	--> func list
	local regain_list = {
		every_time = function( ... )
			return true
		end,
		once_life = function( ... )
			local keyct = string.format("%s:onceAllLife", uid)
			local ok,val = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "srem", keyct, app_name)
			if not ok then return false end
			return true
		end,
		power_on = function( ... )
			local keyct = string.format("%s:oncePowerOn", uid)
			local ok,val = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "srem", keyct, app_name)
			if not ok then return false end
			return true
		end,
		one_day = function( ... )
			local keyct = string.format("%s:%s:everyDay", uid, app_name)
			local ok,val = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", keyct)
			if not ok then return false end
			if val and (tonumber(val) > 0) then
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "decr", keyct)
			end
			--[[
			if delay > 0 then
			local keydy = string.format("%s:%s:everyDelay", uid, app_name)
			redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", keydy)
			end
			]]--
			return true
		end,
		fixed_time = function( ... )
			local keyct = string.format("%s:%s:fixedInterval", uid, app_name)
			local ok,val = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", keyct)
			if not ok then return false end
			if val and (tonumber(val) > 0) then
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "decr", keyct)
			end
			return true
		end,
	}
	return regain_list[ class ]( )
end


function check_is_tb_elem(tb, id)
	for i, v in pairs(tb) do
		if tostring(v) == tostring(id) then
			return true
		end
	end
	return false
end



function is_off_site(app_name)
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local lon = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
	local lat = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
	local ok, home_city = init_data.lru_hash_cache_get("private",accountID, accountID .. ':homeCityCode')
	if not ok or not home_city then
		only.log("I", "can't get homeCityCode from redis!")
		return false
	end
	local ok, city_code = init_data.lru_hash_cache_get('private', accountID, accountID..':cityCode')
	if not ok then
		return false
	end
	if tonumber(city_code) == tonumber(home_city) then
		only.log('D', 'cityCode homeCityCode is same!')
		return false
	end
	return true
end



function is_road_change(app_name)
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local all = #APP_POOL["OUR_BODY_TABLE"]["longitude"]
	local lon_new = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
	local lat_new = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
	local lon_old = APP_POOL["OUR_BODY_TABLE"]["longitude"][all]
	local lat_old = APP_POOL["OUR_BODY_TABLE"]["latitude"][all]
	--[[
	if not weibo.check_driview_subscribed_msg(accountID, weibo.DRI_APP_TRAFFIC_REMIND) then
	only.log('D','高德路况，被客户禁止!')
	return false;
	end
	--]]

	local keyct = string.format("%s:%s:lastPassRoadID", accountID, app_name)
	local ok, last_roadRootID = redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", keyct)
	if not ok then
		return false
	end
	-->get new
	local res = get_road_info(lon_new, lat_new, nil)
	if(tonumber(res['ERRORCODE']) ~= 0) then
		only.log('D', "get road Information failed")
		return false
	end
	local roadRootID_new = res['RESULT']['roadRootID']
	if not roadRootID_new or roadRootID_new == "" then
		only.log('D', "no roadRootID")
		return false
	end
	if roadRootID_new == last_roadRootID then
		return false
	end
	-->get old
	res = get_road_info(lon_old, lat_old, nil)
	if(tonumber(res['ERRORCODE']) ~= 0) then
		only.log('D', "get road Information failed")
		return false
	end
	local roadRootID_old = res['RESULT']['roadRootID']
	if not roadRootID_old or roadRootID_old == "" then
		only.log('D', "no roadRootID")
		return false
	end
	if roadRootID_new ~= roadRootID_old then
		return false
	end
	-->set new
	redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "set", keyct, roadRootID_new)

	only.log('D', string.format("ROAD CHANGE: last %s : old %s to new %s", last_roadRootID, roadRootID_old, roadRootID_new))
	return true
end

function check_time_is_between_in(app_name)
	local gps_time = APP_POOL["OUR_BODY_TABLE"]["GPSTime"][1]
	------check time is between_in ( time_start,time_end )
	local time_start = APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["check_time_is_between_in"]["time_start"]
	local time_end = APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["check_time_is_between_in"]["time_end"]

	local get_gps_time = ( tonumber(gps_time) + 8*60*60 ) % ( 24*60*60 )


	if time_start < time_end then
		----if between 10 ~ 14 
		----then time_start = 10 * 60 * 60 , time_end = 14 * 60 * 60 
		---- 10 < gps_time and gps_time < 14 
		if time_start < get_gps_time   and  get_gps_time < time_end  then
			return true
		else
			return false
		end
	elseif time_start > time_end then
		----if between 23 ~ 4  jmp new day 
		----then time_start = 23 * 60 * 60  ,time_end = 4 *60 * 60
		---- 23 < gps_time or  gps_time < 4 
		if time_start < get_gps_time  or get_gps_time < time_end then
			return true
		else
			return false
		end
	end
	return false
end

function check_speed_is_less_than(app_name)
	local speed_tab = APP_POOL["OUR_BODY_TABLE"]["speed"]
	local speed = APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["check_speed_is_less_than"]["speed"]

	local low_speed
	if #speed_tab == 1 then
		low_speed = speed_tab[1]
	else
		for i = 1, #speed_tab - 1 do 
			low_speed = speed_tab[1]
			if low_speed > speed_tab[i + 1] then
				low_speed = speed_tab[i + 1]
			end
		end
	end
	only.log('D','speed:' .. low_speed)
	if tonumber(low_speed) < tonumber(speed) then
		return true
	else
		return false
	end
end

local function reach_another_step(app_name, accountID, idx_key, index)
	local keyct1 = string.format("%s:onceStepKeysSet", accountID)
	local keyct2 = string.format("%s:%s:onceStepSet", accountID, app_name)
	redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"],"sadd",keyct1,keyct2)
	local ok,val = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sismember", keyct2, index)
	if not ok then return false end
	if not val then
		if idx_key then
			local keyct0 = string.format("%s:%s", accountID, idx_key)
			redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "set", keyct0, index)
		end
		redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sadd", keyct2, index)
		return true
	else
		return false
	end

end


function check_date(ok_cur)
	local cur_date = os.date("%Y%m") .. "01"
	if tostring(ok_cur) >= tostring(cur_date) then
		return true;
	end
	return false;
end


--函数:drive_online_point
--功能:
--说明:目前只有只有疲劳驾驶应用在使用
function drive_online_point(app_name)
	local idx_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["drive_online_point"]["idx_key"]
	local increase = APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["drive_online_point"]["increase"]
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local online = init_data.init_current_online_time
	local divisor = math.floor(online / increase)
	local remainder = online % increase
	if divisor == 0 then
		return false
	end
	only.log("D", string.format("==drive_online_point== %d:%d", divisor,remainder))
	if (remainder > 0) and (remainder < 60) then
		return reach_another_step(app_name, accountID, idx_key, divisor)
	else
		return false
	end
end


function user_control(app_name) 
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local ctl = weibo.DRI_APP_LIST[app_name]
	if not ctl then
		return true;
	end
	if not weibo.check_driview_subscribed_msg(accountID, ctl.no) then
		only.log('D',ctl.text)
		return false;
	end
	return true;
end

--名 称:is_continuous_driving_mileage_point
--功 能:连续驾驶的业务逻辑
--参 数:app_name 应用名字
--返回值:符合要求返回true,不符合返回false
--修 改:修改业务　程少远　2015/05/14
function is_continuous_driving_mileage_point(app_name)
	only.log("D", string.format("[is_continuous_driving_mileage_point]"))
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local increase = APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["is_continuous_driving_mileage_point"]["increase"]
	increase  =  increase and tonumber(increase) or 10



	local sum_point = nil;
	local actual_point = nil;

	--获取有效里程和实际里程
	local hash_table_name = init_data.init_data_imei
	hash_table_name = hash_table_name .. ':'..APP_POOL['OUR_BODY_TABLE']['tokenCode']
	hash_table_name = hash_table_name .. ':' ..'mileage'
	--[[
	local ok, sumMileage = redis_api.cmd('Mileage', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget',hash_table_name, 'sumMileageMts')
	if not ok then
	return false
	end
	]]--
	local ok, actualMileage = redis_api.cmd('Mileage', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget',hash_table_name, 'actualMileageMts')
	if not ok then
		return false
	end

	local ok,maxSpeed = redis_api.cmd('Mileage', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget' , hash_table_name, 'maxSpeed')
	if not ok then
		return false
	end

	local ok,avgSpeed = redis_api.cmd('Mileage', APP_POOL["OUR_BODY_TABL"]["accountID"], 'hget' , hash_table_name, 'avgSpeed')
	if not ok then
		return false
	end
	local ok,stopTime = redis_api.cmd('Mileage', APP_POOL["OUR_BODY_TABL"]["accountID"], 'hget', hash_table_name, 'stopTime')
	if not ok then
		return false
	end
	if  (not actualMileage) and (not maxSpeed) and (not avgSpeed) and (not stopTime)then
		return false
	end


	local sum_repeat_filter_key = accountID ..":sumcontinuousDrivingRepeatFilter"
	local actual_repeat_filter_key = accountID ..":actualcontinuousDrivingRepeatFilter"
	local carry_key = accountID..":continuousDrivingCarryData"

	--local sum_divisor = math.floor(sumMileage / 1000 / increase)
	local actual_divisor = math.floor(actualMileage / 1000 / increase)
	--only.log('D', 'sum_divisor is :' .. tostring(sum_divisor))
	only.log('D', 'actual_divisor is :' ..tostring(actual_divisor))

	--local sum_remainder = (sumMileage / 1000)  % increase
	local actual_remainder = (actualMileage /1000) % increase

	--only.log('D', 'sum_remainder is :' .. tostring(sum_remainder))
	only.log('D', 'actual_remainder is :' ..tostring(actual_remainder))



	if actual_divisor == 0 then
		return false
	end
	--only.log('D',"get sumMileage from redis, value is ".. tostring(sumMileage))
	only.log('D',"get actualMileage from redis, value is " .. tostring(actualMileage))
	--实际里程
	if(actual_remainder >= 0) and (actual_remainder < 1) then
		local ok, val = redis_api.cmd('driview', APP_POOL['OUR_BODY_TABLE']['accountID'], 'sismember', actual_repeat_filter_key, actual_divisor)
		if (ok) and (not val) then 
			only.log('D', "actualMileage "..tostring(actualMileage) .. ',and will broadcast') 
			actual_point = 1
		elseif val then
			only.log('D',"sumMileage is ".. tostring(actualMileage) .. ',and has broadcast')
		end
	end
	--有效里程
	--[[
	if(sum_remainder >= 0) and (sum_remainder < 1) then
	local ok, val = redis_api.cmd('driview', APP_POOL['OUR_BODY_TABLE']['accountID'], 'sismember', sum_repeat_filter_key, sum_divisor)
	if (ok) and (not val) then
	only.log('D',"actualMileage is " .. tostring(actualMileage) .. ',and will broadcast')
	sum_point = 1
	elseif val then
	only.log('D', "actualMileage is " .. tostring(actualMileage) .. ',and hash broadcast')
	end
	end
	]]--
	--构造传递的数据
	local carry_data = nil;
	--[[
	if sum_point and actual_point then
	carry_data = string.format("%s:%s:%s:%s:%s:%s",3, sum_divisor * increase, actual_divisor* increase, maxSpeed, avgSpeed, stopTime)
	elseif (sum_point) and (not actual_point) then
	carry_data = string.format("%s:%s:%s:%s:%s:%s",1, sum_divisor * increase, 0, maxSpeed, avgSpeed, stopTime)
	elseif(not sum_point) and actual_point then
	carry_data = string.format("%s:%s:%s:%s:%s:%s", 2, 0, actual_divisor * increase , maxSpeed, avgSpeed, stopTime)
	else
	only.log('D', "sumMileage and actualMileage are not required, so will not broadcast")
	return false
	end
	]]--
	if actual_point then
		carry_data = string.format("%s:%s:%s:%s:%s:%s", 2, 0, actual_divisor * increase , maxSpeed, avgSpeed, stopTime)
	else
		only.log('D', "sumMileage and actualMileage are not required, so will not broadcast")
		return false
	end
	redis_api.cmd('driview', APP_POOL['OUR_BODY_TABLE']['accountID'], 'set', carry_key, carry_data)
	--redis添加本次播放
	--[[
	if sum_point then
	redis_api.cmd('driview', APP_POOL['OUR_BODY_TABLE']['accountID'], 'sadd', sum_repeat_filter_key,sum_divisor)
	end
	]]--

	if actual_point then
		redis_api.cmd('driview', APP_POOL['OUR_BODY_TABLE']['accountID'], 'sadd', actual_repeat_filter_key, actual_divisor)
	end

	return true;
end





function is_over_speed(app_name)
	local accountID		= APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local direction		= APP_POOL["OUR_BODY_TABLE"]["direction"] and APP_POOL["OUR_BODY_TABLE"]["direction"][1] or -1
	local increase		= APP_CONFIG_LIST["OWN_LIST"][app_name]["bool"]["is_over_speed"]["increase"]
	increase		= increase and tonumber(increase) or 120
	local accountID		= APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local all		= #APP_POOL["OUR_BODY_TABLE"]["longitude"]
	local over_speed_carry	= accountID .. ":overSpeedCarry"
	local is_over_speed_start_time_key	= accountID .. ":isOverSpeedStartTime"
	local is_over_speed_point_count_key	= accountID .. ":isOverSpeedPointCount"
	-->> check speed
	local speed				= APP_POOL["OUR_BODY_TABLE"]["speed"][1]
	if not speed or tonumber(speed) == 0 then
		return false
	end
	-->> check lon lat
	local lon		= APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
	local lat		= APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
	if not lon or not lat then
		return false
	end
	-->> get point info
	local res = init_data.point_match_road_result
	if not res then
		return false
	end
	local limit_speed	= res['SR']
	local rt		= tonumber(res['RT'])
	only.log('D', scan.dump(res))
	only.log('D', string.format("[rt = %s]", rt))

	-->> fiter ok limit road
	if not limit_speed or limit_speed == "NULL" then
		return false
	end
	limit_speed = tonumber(limit_speed)
	if limit_speed == 0 then
		return false
	end
	if rt == 0 and limit_speed < 80 then
		return false
	end
	if rt ~= 0 and limit_speed < 60 then
		return false
	end
	only.log('D', string.format("[limit_speed:%s]", limit_speed))

	-->> get over speed point count
	local high_limit_speed = limit_speed * 1.05
	local cnt = 0
	for i = 1 , all do
		speed = APP_POOL["OUR_BODY_TABLE"]["speed"][i]
		if speed > high_limit_speed then
			cnt = cnt + 1
		end
	end

	local ok, point_count = redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', is_over_speed_point_count_key)
	if not ok then
		only.log('D', "[redis is error]")
		return false
	end
	local ok, start_time = redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', is_over_speed_start_time_key)
	if not ok then
		only.long("D", "[redis is error]")
		return false
	end
	point_count = tonumber(point_count) or -1
	start_time = tonumber(start_time ) or -1
	local time = os.time()

	if cnt == 5 and (point_count == -1 or start_time + 180 < time) then
		redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', is_over_speed_point_count_key, 0)
		redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', is_over_speed_start_time_key, time)
		local flag = 0
		local value = string.format("%s:%s", flag, limit_speed)
		--print(string.format("[value:%s]", value))
		--only.log('D', string.format("[value:%s]", value))
		only.log('D', string.format("[over_speed][value:%s][limit_speed:%s]", 
		value,limit_speed))
		redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', over_speed_carry, value)
		return true
	end

	if start_time + 180 >  time then
		local over_speed_count = 0
		for i = 1 , all do
			if tonumber(APP_POOL["OUR_BODY_TABLE"]["speed"][i]) > high_limit_speed  then
				over_speed_count  = over_speed_count  + 1
			end
		end
		over_speed_count =  over_speed_count  + point_count

		--local NUM = 30
		local NUM = 45
		--local NUM = 60
		if over_speed_count >= NUM  then
			redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', is_over_speed_point_count_key, 0)
			redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', is_over_speed_start_time_key, time)
			local flag = 1
			local value = string.format("%s:%s", flag , limit_speed)
			--print(string.format("[value:%s]", value))
			--only.log('D', string.format("[value:%s]", value))
			only.log('D', string.format("[over_speed][value:%s][limit_speed:%s]", 
			value,limit_speed))
			redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', over_speed_carry, value)
			return true;
		else
			redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', is_over_speed_point_count_key, over_speed_count )
		end
	end


	return false
end

function is_weather_forcast(app_name)
	only.log('D', string.format('[is_weather_forcast]'))
	local accountID             = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local weather_forcast_key   = accountID .. ":weatherForecast"

	--[[
	local test_accountID = {
	["yHnmlqIW9Q"]   =  { txt =   "建添" } , 
	["DmBuB45EbZ"]   =  { txt =   "何桑" } ,
	["kxl1QuHKCD"]   =  { txt =   "晓天" } ,
	["pQvEPywNzY"]   =  { txt =   "王车贵"},
	}
	if not test_accountID[accountID] then
	return false
	end
	--]]

	--local ok, ret = redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", weather_forcast_key)
	--local ok, ret = init_data.lru_cache_get( weather_forcast_key)
	local ok, ret = init_data.lru_hash_cache_get("driview",accountID, weather_forcast_key)
	if not ok then 
		only.log('E', 'redis operation error')
		return false 
	end
	ret = ret and tonumber(ret) or 0
	only.log("D", string.format("[ret:%s]", ret))
	if ret == 1 then
		return false
	end
	if ret == 0 then
		--redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "set", weather_forcast_key, 1)
		--init_data.lru_cache_set(weather_forcast_key, '1', 1)
		init_data.lru_hash_cache_set("driview",accountID,weather_forcast_key, '1', 1)
		return true
	end
	return false
end

--名  称：get_city_code
--功  能：通过经纬度获取城市到代码
--参  数：accountID
--返回值：城市代码
function get_city_code()
	local longitude = APP_POOL["OUR_BODY_TABLE"]["altitude"][1]
	local latitude  = APP_POOL["OUR_BODY_TABLE"]["altitude"][1]
	local grid = string.format('%d&%d',tonumber(longitude)*100,tonumber(latitude)*100)
	only.log('D',"Grid string :" .. tostring(grid))
	local ok,jo =  redis_api.cmd('mapGridOnePercent',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',grid)
	only.log('D',"cityCode :" .. tostring(jo))
	if ok and jo then
		local ok,info = pcall(cjson.decode,jo)
		if not ok or not info then
			only.log('E',"json result error!-->" .. info)
		else
			local cityCode = info["cityCode"]
			if cityCode then
				return cityCode
			end
		end
	end
end
