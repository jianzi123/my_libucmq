local redis_api         = require('redis_pool_api')
--local route             = require('route')
local cjson             = require('cjson')
local logs              = require('logs')
local APP_POOL          = require('pool')
local model             = require('model')
local utils             = require('utils')
local only              = require('only')
local socket            = require('socket')
local link              = require('link')
local supex             = require('supex')
local scan              = require('scan')
local cfg               = require('cfg')
local libkv		= require('libkv')
local driview_fun_point_match_road = require ('driview_fun_point_match_road')
local serv_name = app_lua_get_serv_name()
--local lru               = require('lru')

module('init_data', package.seeall)


local longitude_tb 
local latitude_tb 
local speed_tb
local direction_tb
local altitude_tb 
local gpstime_tb
local lon       
local lat       
local speed     
local dir       
local alt       
local gpstime   
local num
local accountID 


NAME = "init_data"
point_match_road_result = nil
road_limit_speed = nil
user_msg_subscribed = nil
init_data_imei = nil
init_current_online_time = 0            --是数字



local function lru_cache_test()
	local key = "aaa"
	local ok, value = app_lua_lru_cache_get_value(key)
	only.log('D', string.format("[get value][ok:%s][value:%s]", ok, value))
	if not ok then
		local value = "bbb"
		local ok, ret = app_lua_lru_cache_set_value(key, value, #value)
		if not ok then
			only.log('D', string.format("[set value :ret :%s]\n", ret))
		end
	end
end


function get_imei ( accountID )
	if #accountID == 15 then
		return accountID
	end
	local key = accountID ..":IMEI"
	local ok, value = app_lua_lru_cache_get_value(key) 
	only.log('D', string.format("[get value][key:%s][ok:%s][value:%s]", key or '', ok or '', value or ''))
	if not ok then
		ok,value= redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', key)
		only.log('D', string.format('[get_imei][ok:%s][value:%s]', ok or '', value or ''))
		if ok and value then
			local ok, ret = app_lua_lru_cache_set_value(key, value, #value)
			if not ok then
				only.log('D', string.format("[set value :ret :%s]\n", ret or ''))
			end
		else
			only.log('I', "get init_data_imei error")
			return nil
		end
	end
	return value
end
--函数:get_current_online_time
--功能:计算当前用户的在线时间
local function get_current_online_time(accountID)
        local key = accountID..':configTimestamp'
        local ok, cfg_time = init_data.lru_hash_cache_get("driview",accountID,key)
	if ok and cfg_time then
                local timestamp = os.time()
                local value = timestamp - cfg_time
                return value
        end
        return 0
end
local function get_user_msg_subscribed (accountID) 
	local key = accountID ..":userMsgSubscribed"
	local ok, value = app_lua_lru_cache_get_value(key) 
	only.log('D', string.format("[get value][key:%s][ok:%s][value:%s]", key or '', ok or '', value or ''))
	if not ok then
		ok,value= redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', key)
		if ok and value then
			local ok, ret = app_lua_lru_cache_set_value(key, value, #value)
			if not ok then
				only.log('D', string.format("[set value :ret :%s]\n", ret or ''))
			end
		else
			--only.log('E', "[get_user_msg_subscribed] get user_msg_subscribed error")
			value = nil
		end
	end
	return value
end
        
        
function lru_hash_cache_set(server, hash, key,value, value_len)
        only.log('D', string.format("[server:%s][hashkey:%s][key:%s][value:%s][value_len:%s]", server or '', hash or '', key or '', value or '', value_len or ''))
	redis_api.cmd(server,hash,'set', key, value)
	local ok, ret = app_lua_lru_cache_set_value(key, value, value_len)
	if not ok then
		only.log('D', string.format("[set value cache error][ret :%s]\n", ret or ''))
	end
end
     
function lru_hash_cache_get (server, hash, key)
	local ok, value = app_lua_lru_cache_get_value(key) 
	only.log('D', string.format("[server:%s][hashkey:%s][get value][key:%s][ok:%s][value:%s]", server or '', hash or '',key or '', ok or '', value or ''))
	if not ok then
		ok,value= redis_api.cmd(server,hash, 'get', key)
		if ok and value then
			local ok, ret = app_lua_lru_cache_set_value(key, value, #value)
			if not ok then
				only.log('D', string.format("[set value :ret :%s]\n", ret or ''))
			end
		else
			only.log('I', string.format("get value of [key:%s] from redis error", key or ''))
			value = nil
		end
	end
	return true, value
end

function lru_cache_set (key , value, value_len)
	only.log('D', string.format("[key:%s][value:%s][value_len:%s]", key, value, value_len))
	redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', key, value)
	local ok, ret = app_lua_lru_cache_set_value(key, value, value_len)
	if not ok then
		only.log('D', string.format("[set value cache error][ret :%s]\n", ret or ''))
	end
end



function lru_cache_get (key)
	local ok, value = app_lua_lru_cache_get_value(key) 
	only.log('D', string.format("[get value][key:%s][ok:%s][value:%s]", key or '', ok or '', value or ''))
	if not ok then
		ok,value= redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', key)
		if ok and value then
			local ok, ret = app_lua_lru_cache_set_value(key, value, #value)
			if not ok then
				only.log('D', string.format("[set value :ret :%s]\n", ret or ''))
			end
		else
			only.log('I', string.format("get value of [key:%s] from redis error", key or ''))
			value = nil
		end
	end
	return true, value
end

function speed_statistics()
	for i = 1, num do
		local speed = speed_tb[i]
		local speed_tb = {
			[0] = '00',
			[1] = '01',
			[2] = '02',
			[3] = '03',
			[4] = '04',
			[5] = '05',
			[6] = '06',
			[7] = '07',
			[8] = '08',
			[9] = '09',
			[10] = '10',
			[11] = '11',
			[12] = '12',
			[13] = '13',
			[14] = '14',
			[15] = '15',
		}

                local ok,travelID = app_lua_lru_cache_get_value(APP_POOL["OUR_BODY_TABLE"]["accountID"] .. ':travelID')
                if (not ok ) or (not travelID) then
                        ok, travelID = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:travelID", init_data_imei ))
                end
		--only.log('D', string.format("[i = %d][ok_status = %s][travelID = %s]", i, ok_status, travelID))
		local speed_mod = math.floor(speed/10)
		if speed_mod > 15 then
			speed_mod = 15
		end
		local speed_key = speed_tb[speed_mod]
                --[[
		local ok_date, cur_date = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:speedDistribution", accountID))
		if not ok_date or not cur_date then
			cur_date = os.date("%Y%m")
			redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'set', string.format("%s:speedDistribution", accountID), cur_date)
		end
                ]]--
                local cur_date = os.date("%Y%m")
		if ok_status and travelID and speed_key and cur_date then
			local datacore_statistics_var_key  = accountID .. ":" .. travelID .. ":speedDistribution"  .. ":" .. cur_date
			--only.log('D', string.format("[i = %d][key = %s]", i, datacore_statistics_var_key))
			redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'hincrby', datacore_statistics_var_key, speed_key, 1)
			redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'expire', datacore_statistics_var_key, 48*3600)
		end
	end
end





local function get_road_info(lon, lat, dir, accountID)
	local ok, ret = driview_fun_point_match_road.entry(dir,lon,lat,accountID)
        --local ok, ret = entry(dir,lon,lat, accountID)
	if ok and ret then
		local roadID = ret['roadID']
		local lineID = ret['lineID']
		local kv_tab
		local key = roadID .. ":roadInfo" 
		local ok, result = redis_api.cmd("mapRoadInfo",APP_POOL["OUR_BODY_TABLE"]["accountID"], "hgetall", key)
		if ok and result then
			kv_tab = result
			kv_tab['roadID'] = roadID
			kv_tab['lineID'] = lineID
			only.log("D", string.format("[result:%s]", scan.dump(kv_tab)))
		else
			only.log('E', "get mapRoadInfo failed");
		end
		only.log('D',"[[Function:get_road_info]] over")
		return kv_tab
	else
		only.log('I', "Point Match Road ERROR")
		return nil
	end
end

--函数:get_stop_time
--功能:计算停止次数
--说明:停止计算
--[[
function get_stop_time()
	local stop_time = 0
	for i = 1, num do
		if tonumber(speed_tb[i]) == 0 then
			stop_time = stop_time + 1
		end
	end
	return stop_time
end
]]--
--函数:get_low_speed_mileage
--功能:计算低速行驶里程
--说明：已经放弃低速里程的计算
--[[
function get_low_speed_mileage(low_limit_speed)
	local low_speed_mileage = 0
	local threshold = 40
	if tonumber(num) == 1 then
		return 0
	end
	for i=1,(num -1) do
		local between = gpstime_tb[i] - gpstime_tb[i+1]
		if (direction_tb[i] ~= "-1") and (direction_tb[i+1] ~= "-1") and
			(between < 11) and (between > 0) and 
			speed_tb[i] <  low_limit_speed then
			local 	mileage = ((speed_tb[i] + speed_tb[i+1]) / 2) * between
			if mileage > threshold  then
				--- do nothing
			else
				low_speed_mileage = low_speed_mileage  + mileage
			end
		end
	end
	return math.floor(low_speed_mileage/(3.6)) -- m
end
]]--
--函数:get_over_speed_mileage
--功能:计算高速行驶的里程
--说明:该功能已经不需要
--[[
function (high_limit_speed)
	local over_speed_mileage = 0
	local threshold = 40
	if num == 1 then
		return 0
	end
	for i=1,(num -1) do
		local between = gpstime_tb[i] - gpstime_tb[i+1]
		if (direction_tb[i] ~= "-1") and (direction_tb[i+1] ~= "-1") and
			(between < 11) and (between > 0) and 
			speed_tb[i] >  high_limit_speed then
			local 	mileage = ((speed_tb[i] + speed_tb[i+1]) / 2) * between
			if mileage > threshold  then
				--- do nothing
			else
				over_speed_mileage = over_speed_mileage  + mileage
			end
		end
	end
	return math.floor(over_speed_mileage/(3.6))
end
]]--
function init_point_info ()
        --[[
	local low_speed_mileages_key        = accountID .. ":lowSpeedMileages"
	local over_speed_mileages_key       = accountID .. ":overSpeedMileages"
	local stop_time_key                 = accountID .. ":stopTimes"

	-- sum stop time
	local stop_time = get_stop_time()
	if stop_time >  0 then
		local ok, sum_stop_time = redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'incrby', 
		stop_time_key, stop_time)
		if not ok then
			only.log('D', "[redis is error]")
			return false
		end
		sum_stop_time = tonumber(sum_stop_time) or 0
		only.log('D', string.format(
		"[init_point_info][stop_time:%s][sum_stop_time:%s]", 
		stop_time ,sum_stop_time))
	end

	--sum low speed mileages
	local low_limit_speed = 20
	local low_speed_mileage = 0
	low_speed_mileage  = get_low_speed_mileage(low_limit_speed)
	if low_speed_mileage >  0 then
		local ok, sum_mileages = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'incrby', 
		low_speed_mileages_key, low_speed_mileage)
		if not ok then
			only.log('D', "[redis is error]")
			return false
		end
		sum_mileages  = tonumber(sum_mileages) or 0
		only.log('D', string.format(
		"[init_point_info][low_speed_mileage:%s][sum_mileages:%s]", 
		low_speed_mileage ,sum_mileages))
	end
        ]]--
	---- over speed 
	--local res = get_road_info(lon, lat, "2", direction)
        --[[
	res = point_match_road_result
	if not res then
		only.log('D', "get road Information failed")
		return false
	end

	local limit_speed = nil
	--按照限速信息来做
	local limitSpeed = res['SR']

	limit_speed = limitSpeed

	if not limit_speed or limit_speed == "NULL" 
		or tonumber(limit_speed) == 0 then
		--按照道路等级来做
		local roadType = res['RT']
		local road_tb = { 
			[0] = {txt = "高速", speed = 120},
			[1] = {txt = "国道", speed= 80},
			[2] = {txt = "省道", speed = 60},
			[3] = {txt = "县道", speed = 50},
			[4] = {txt = "乡道", speed = 40},
			[5] = {txt = "村道", speed = 30},
			[7] = {txt = "普通道路", speed = 40},
			[10] = {txt = "城市快速路", speed = 80},
			[11] = {txt = "城市主干道", speed = 60},
			[12] = {txt = "城市次干道", speed = 50},
			[15] = {txt = "步行街", speed = 0},
			[16] = {txt = "内部道路", speed = 5},
		}
		local mid  = road_tb[tonumber(roadType)] 
		limit_speed =  mid and mid['speed'] or 60
	end

	limit_speed = tonumber(limit_speed)
	only.log('D', string.format("[limit_speed:%s]", limit_speed))
	road_limit_speed = limit_speed 
	local high_limit_speed = limit_speed * (1 + 0.05)
        ]]--
	--sum over speed mileages
        --[[
	local over_speed_mileage  = get_over_speed_mileage(high_limit_speed)
	if  over_speed_mileage >  0 then
		local ok, sum_mileages = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'incrby', 
		over_speed_mileages_key, over_speed_mileage)
		if not ok then
			only.log('D', "[redis is error]")
			return false
		end
		sum_mileages  = tonumber(sum_mileages) or 0
		only.log('D', string.format(
		"[init_point_info][over_speed_mileage:%s][sum_mileages:%s]", 
		over_speed_mileage ,sum_mileages ))
	end
        ]]--
	return false;
end

function handle ()
	if  serv_name == "appcenter"  then
		return true
	end
        
        

	local t1 = socket.gettime()
	local t2 = nil
	--only.log('D', "[handle]")
	longitude_tb    = APP_POOL["OUR_BODY_TABLE"]["longitude"] 
	latitude_tb     = APP_POOL["OUR_BODY_TABLE"]["latitude"]
	speed_tb        = APP_POOL["OUR_BODY_TABLE"]["speed"] 
	direction_tb    = APP_POOL["OUR_BODY_TABLE"]["direction"] 
	altitude_tb     = APP_POOL["OUR_BODY_TABLE"]["altitude"]
	gpstime_tb      = APP_POOL["OUR_BODY_TABLE"]["GPSTime"]
	accountID       = APP_POOL["OUR_BODY_TABLE"]["accountID"]
        
        --[[
        local ok  ,ret= libkv_hash_cache_set('private',accountID, "testkey", "testvalue")
        if not ok then
                print("libkv_hash_cache_set error")
                print(ret)
        end
       ]]-- 
	--[[
      	while true 
	do
        	ok, ret = libkv.libkv_hash_cache_get('private',accountID,"testabkey")
	        if not ok then
			only.log('D', tostring(ok))
        	        only.log('D',"libkv_hash_cache_get error")
			only.log('D', tostring(ret))
		else
			only.log('D',tostring(ok))
			only.log('D',"libkv_hash_cache_get ok")
			only.log('D', tostring(ret))
       		 end
	end
	]]--
                
	if not accountID then
		return false
	end

	--[[
	local ok_status, ok_val= redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', accountID .. ':userMsgSubscribed')	
	if not ok_status then		
		ok_val = nil		
	end		
	user_msg_subscribed = ok_val
	--]]
	user_msg_subscribed = get_user_msg_subscribed( accountID )

	if not  longitude_tb or not latitude_tb then
		return false
	end

	lon       =  longitude_tb and longitude_tb[1] or 0
	lat       =  latitude_tb  and latitude_tb[1]  or 0
	speed     =  speed_tb     and speed_tb[1]     or 0
	dir       =  direction_tb and direction_tb[1] or '-1'
	alt       =  altitude_tb  and altitude_tb[1]  or 0
	gpstime   =  gpstime_tb   and gpstime_tb[1]   or 0
	num       = #longitude_tb

        --[[
  	if gpstime_tb[1] then
    		local key = accountID .. ':currentBL'
    		--local ok_status, gps_str = init_data.lru_cache_get( 'private', accountID, key )
                local ok_status, gps_str = lru_hash_cache_get('private',accountID, key)
                --local ok_status, gps_str = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', key)
    		if ok_status then
     		 last_gps_info = gps_str
    		end
    
    		key = accountID .. ':currentGPSTime'
                local ok,last_time = lru_hash_cache_get('private', accountID, key)
    		--local ok,last_time = init_data.lru_cache_get( 'private_hash', accountID,key )
                --local ok,last_time = redis_api.cmd('private_hash',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', key)
    		if ok then
      			last_gps_time = last_time
    		end
    			update_bl_and_time = 1
  		else
		    	last_gps_info = nil
    		    	last_gps_time = nil
    			update_bl_and_time = 0
  	end
        ]]--
        
	local key = 'gps_count'
	local ok, value = app_lua_lru_cache_get_value(key) 
	if ok then
		value = value + 1
		value_len = # tostring(value)
		app_lua_lru_cache_set_value(key, value, value_len)
	else
		value = 1
		value_len = # tostring(value)
		app_lua_lru_cache_set_value(key, value, value_len)
	end
	only.log('D', string.format('[gps_count:%s]', value or 0))


	point_match_road_result = get_road_info(lon, lat, dir,accountID)
	--only.log('D', string.format("[point_match_road_result:%s]", scan.dump(point_match_road_result)))
	t2 = socket.gettime()
	init_point_info()

	--[[
	ok,init_data_imei= redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':IMEI')	
	if not ok or not init_data_imei then	
		only.log('E', "[init_data][get init_data_imei error]")
	end
	--]]
	lru_cache_test()
	init_data_imei = get_imei (accountID)

        init_current_online_time = get_current_online_time(accountID)
	--[[
	local ok, gps = init_data.lru_cache_get(accountID .. ':trafficBL')
	if not ok then
		local value = string.format("%s,%s",longitude_tb[1], latitude_tb[1] )
		local value_len = #value
		init_data.lru_cache_set(accountID .. ':trafficBL',value, value_len)
		gps = value
	end
	only.log('D', string.format('[gps:%s]', gps))
	--]]


	local t3 = socket.gettime()
	--
	--speed_statistics()
        if(cfg["OWN_INFO"]["SYSLOGLV"]) then
                only.log('S', string.format("MODULE : %s ===> ifon=(%s) insp=(%s) call=(%s) match [%f] | work [%f] | total [%f]",
                "init_data", ifon, insp, state, ifon and (t2 - t1) or 0, ifon and (t3 - t2) or 0, t3 - t1))
        end
end
