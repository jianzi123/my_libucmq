local APP_POOL              = require("pool")
local http_api              = require('http_short_api')
local redis_api             = require("redis_pool_api")
local only                  = require("only")
local utils                 = require("utils")
local cutils                = require("cutils")
local supex                 = require("supex")
local cjson                 = require("cjson")
local weibo                 = require("weibo")
local init_data             = require("init_data")
local fun_point_match_road  = require ('fun_point_match_road')
local scan                  = require('scan')
local APP_JUDGE = require("judge")


module("l_f_road_traffic", package.seeall)

function bind()
	return '["longitude","latitude","accountID","GPSTime","direction","collect","speed"]'
end



local function mileage_raise( )
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local longitude = APP_POOL["OUR_BODY_TABLE"]["longitude"]
	local latitude = APP_POOL["OUR_BODY_TABLE"]["latitude"]
	local GPSTime = APP_POOL["OUR_BODY_TABLE"]["GPSTime"]
	local direction = APP_POOL["OUR_BODY_TABLE"]["direction"]
	local speed = APP_POOL["OUR_BODY_TABLE"]["speed"]

	local all = #GPSTime
	local mileage = 0
	if all == 1 then
		return
	end
	--local ok,last_long = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':trafficLongitude')
	--local ok,last_lat = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':trafficLatitude')
	--local ok_status, gps_str = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',accountID .. ':trafficBL')
	--local ok_status, gps_str = init_data.lru_cache_get(accountID .. ':trafficBL')
	local last_long,last_lat = 0, 0
        local gps_info
	if init_data.last_gps_info then
		gps_info = utils.str_split(init_data.last_gps_info,",")
		last_long      = gps_info[1]
		last_lat       = gps_info[2]
	end

	--local ok,last_time = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':trafficGPSTime')
	--local ok, last_time = init_data.lru_cache_get(accountID .. ':trafficGPSTime')
        local last_time = init_data.last_gps_time

	last_time = last_time or 0;
	if last_long and last_lat and os.time() < (last_time + 300) then
		mileage = cutils.gps_distance(last_long, last_lat, longitude[all], latitude[all])
	end
	for i=1,(all -1) do
		local between = GPSTime[i] - GPSTime[i+1]
		if (direction[i] ~= "-1") and (direction[i+1] ~= "-1") and (between < 11) and (between > 0) then
			mileage = mileage + ((speed[i] + speed[i+1]) / 2) * between
		end
	end
	local add_mileage = mileage / (3600)
	if add_mileage > 0 then
		redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'incrbyfloat', accountID .. ':trafficMileage', add_mileage)
	end

	--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':trafficLongitude', longitude[1])
	--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':trafficLatitude', latitude[1])
	--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':trafficBL', string.format("%s,%s",longitude[1], latitude[1] ))
	--local value = string.format("%s,%s",longitude[1], latitude[1] )
  
  	if(init_data.update_bl_and_time == 1)then
    		local gps_value = longitude[1]..","..latitude[1]
    		local value_len = #gps_value
    		--init_data.lru_cache_set(accountID..':currentBL', gps_value, value_len)
    		init_data.lru_hash_cache_set("private",accountID,accountID..':currentBL', gps_value, value_len)
		only.log('D', 'gps_value is:' .. gps_value)
    		--init_data.lru_cache_set(accountID .. ':trafficBL',value, value_len)
    		--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':trafficGPSTime', GPSTime[1])
    		--init_data.lru_cache_set(accountID..':currentGPSTime', GPSTime[1], #(tostring(GPSTime[1])))
    		init_data.lru_hash_cache_set("private",accountID,accountID..':currentGPSTime', GPSTime[1], #(tostring(GPSTime[1])))
		
    		init_data.update_bl_and_time = 0
    		--init_data.lru_cache_set(accountID .. ':trafficGPSTime', GPSTime[1], #(tostring(GPSTime[1])))
    		only.log('D',"l_f_road_traffic mileage_raise update,old gps:" ..init_data.last_gps_info .." ,old time :"..tostring(last_time) ..",new_gps_info:"..gps_value..",new time:" .. tostring(GPSTime[1]))
  	end
end

local function mileage_clean( app_name )
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local longitude = APP_POOL["OUR_BODY_TABLE"]["longitude"]
	local latitude = APP_POOL["OUR_BODY_TABLE"]["latitude"]
	local GPSTime = APP_POOL["OUR_BODY_TABLE"]["GPSTime"]

	local keyct = string.format("%s:%s:onceStepSet", accountID, app_name)
	redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", keyct)
	redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':trafficMileage', 0)

	--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':trafficLongitude', longitude[1])
	--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':trafficLatitude', latitude[1])
	--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':trafficBL', string.format("%s,%s",longitude[1], latitude[1] ))
  	if(init_data.update_bl_and_time == 1) then
		--local value = string.format("%s,%s",longitude[1], latitude[1] )
    		local gps_value = longitude[1]..","..latitude[1]
    		local value_len = #gps_value
    		--init_data.lru_cache_set(accountID..':currentBL',gps_value, value_len)
    		init_data.lru_hash_cache_set("private",accountID,accountID..':currentBL',gps_value, value_len)
    		--init_data.lru_cache_set(accountID..':trafficGPSTime',GPSTime[1], #(tostring(GPSTime[1])))
    		init_data.lru_hash_cache_set("private",accountID,accountID..':currentGPSTime',GPSTime[1], #(tostring(GPSTime[1])))
    		init_data.update_bl_and_time = 0
    		only.log('D',"l_f_road_traffic mileage_clean update,old gps:" ..init_data.last_gps_info .." ,old time :"..tostring(init_data.last_gps_time) ..",new_gps_info:"..gps_value..",new time:" .. tostring(GPSTime[1]))
 	end
  
    --init_data.lru_cache_set(accountID .. ':trafficBL',value, value_len)
    --redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':trafficGPSTime', GPSTime[1])
    --init_data.lru_cache_set(accountID .. ':trafficGPSTime', GPSTime[1], #(tostring(GPSTime[1])))
end


local function get_road_info(lon, lat, dir, accountID)
	local ok, ret = fun_point_match_road.entry(dir,lon,lat,accountID)
        --local ok, ret = init_data.entry(dir,lon,lat,accountID)
	if ok and ret then
		local roadID = ret['roadID']
		local lineID = ret['lineID']
		local kv_tab
		local key = roadID .. ":roadInfo" 
		local ok, result = redis_api.cmd("mapRoadInfo",APP_POOL["OUR_BODY_TABLE"]["accountID"], "hgetall", key)
		--only.log('D', string.format("[ok:%s][result:%s]", ok, result))
		if ok and result then
			kv_tab = result
			kv_tab['roadID'] = roadID
			kv_tab['lineID'] = lineID
			--only.log("D", string.format("[result:%s]", scan.dump(kv_tab)))
		else
			only.log('E', "get mapRoadInfo failed");
			return nil
		end
		return kv_tab
	else
		only.log('W', "Point Match Road failed");
		return nil
	end
end

local function is_road_change(app_name)
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local all = #APP_POOL["OUR_BODY_TABLE"]["longitude"]
	local lon_new = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
	local lat_new = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
	local dir_new = APP_POOL["OUR_BODY_TABLE"]["direction"][1] or "-1"
	local lon_old = APP_POOL["OUR_BODY_TABLE"]["longitude"][all]
	local lat_old = APP_POOL["OUR_BODY_TABLE"]["latitude"][all]
	local dir_old = APP_POOL["OUR_BODY_TABLE"]["direction"][all] or "-1"
	local now_dir =  -1

	for k,v in pairs( APP_POOL["OUR_BODY_TABLE"]["direction"] or {}) do
		if v ~= -1 then
			now_dir = v
			break
		end
	end

	if now_dir == -1 then
		return false
	end

	local keyct = string.format("%s:%s:lastPassRoadID", accountID, app_name)
	local ok, ret = redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", keyct)
	if not ok then
		return false
	end

	local last_roadRootID, last_dir
	if not ret then 
		ret = "_:-1" 
	end
	local a = string.find(ret , ":") 
	if a then 
		last_roadRootID = string.sub(ret, 1, a - 1) 
		last_dir = string.sub(ret, a + 1) 
	else
		last_roadRootID = ret
		last_dir = -1
	end
	--local last_roadRootID = ret

	-->get new
	local res = init_data.point_match_road_result
	if not res then
		only.log('D', "get road Information failed")
		return false
	end
	local roadRootID_new = res['RN']
	if not roadRootID_new or roadRootID_new == "" then
		only.log('D', "no roadRootID")
		return false
	end

	-->get old
	res = get_road_info(lon_old, lat_old, dir_old, accountID)
	if not res then
		return false
	end

	local roadRootID_old = res['RN']
	if not roadRootID_old or roadRootID_old == "" then
		only.log('D', "no roadRootID")
		return false
	end
	if roadRootID_new ~= roadRootID_old then
		return false
	end

	local dir_abs = math.abs(last_dir - now_dir)
	if roadRootID_new == last_roadRootID then
		if dir_abs < 150  or dir_abs > 210 then
			return false
		end
	end

	if res['RT'] == 10 and roadRootID_new ~= last_roadRootID then
		if dir_abs < 30 or dis_abs > 330 then
			return false
		end
	end

	-->set new
	roadRootID_new = roadRootID_new .. ":" .. now_dir
	redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "set", keyct, roadRootID_new)
	only.log('S', string.format("ROAD CHANGE: last %s : old %s to new %s", ret, roadRootID_old, roadRootID_new))
	return true
end

local function reach_another_step(app_name, accountID, idx_key, index)
	local keyct1 = string.format("%s:onceStepKeysSet", accountID)
	local keyct2 = string.format("%s:%s:onceStepSet", accountID, app_name)
	redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sadd", keyct1, keyct2)

	local ok,val = redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sismember", keyct2, index)
	if not ok then return false end
	if not val then
		if idx_key then
			local keyct0 = string.format("%s:%s", accountID, idx_key)
			redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "set", keyct0, index)
		end
		redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sadd", keyct2, index)
		return true
	else
		return false
	end
end

local function drive_mileage_point(app_name, increase, idx_key)
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local ok, mileage = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':trafficMileage')
	if (not ok) or (not mileage) then
		return false
	end
	local divisor = math.floor(mileage / increase)
	local remainder = mileage % increase
	only.log("D", string.format("==drive_mileage_point== %d:%d", divisor,remainder))
	if divisor == 0 then
		return false
	end
	if (remainder > 0) and (remainder < 1) then
		return reach_another_step(app_name, accountID, idx_key, divisor)
	else
		return false
	end
end

local function get_road_type( )
	local res = init_data.point_match_road_result
	if not res then
		only.log('D', "get road Information failed")
		return false
	end

	local road_type = tonumber( res['RT'] )
	if not road_type then
		only.log('D', "no road level")
	end
	return road_type
end

local function check_time_is_between_in( start_time, stop_time )
	local gps_time = APP_POOL["OUR_BODY_TABLE"]["GPSTime"][1]
	local get_gps_time = ( tonumber(gps_time) + 8*60*60 ) % ( 24*60*60 )

	if start_time < stop_time then
		----if between 10 ~ 14 
		----then start_time = 10 * 60 * 60 , stop_time = 14 * 60 * 60 
		---- 10 < gps_time and gps_time < 14 
		if start_time < get_gps_time   and  get_gps_time < stop_time  then
			return true
		else
			return false
		end
	elseif start_time > stop_time then
		----if between 23 ~ 4  jmp new day 
		----then start_time = 23 * 60 * 60  ,stop_time = 4 *60 * 60
		---- 23 < gps_time or  gps_time < 4 
		if start_time < get_gps_time  or get_gps_time < stop_time then
			return true
		else
			return false
		end
	end
	return false
end


function match()
	only.log('D', "[l_f_road_traffic][match]")
	local app_name = "l_f_road_traffic"
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]

	if not (APP_JUDGE.user_control('l_f_road_traffic')) then
		return false 
	end 

	--[[
	if not weibo.check_driview_subscribed_msg(accountID, weibo.DRI_APP_TRAFFIC_REMIND) then
		only.log('D','高德路况，被客户禁止!')
		return false;
	end
	--]]

	if check_time_is_between_in( 22.5*60*60, 6*60*60 ) then
		return false
	end

	local road_type_list = {
		[1]=1,
		[2]=2,
		[3]=3,
		[4]=4,
		[5]=5,
		[7]=7,
		[10]=10,
		[11]=11,
		[12]=12,
		[16]=16,
	}

	if is_road_change( app_name ) then
		mileage_clean( app_name )
		return true
	else
		mileage_raise( )
		--> get road_type
		local road_type = get_road_type( )
		if road_type then
			--> check postion
			--if (road_type >= 1) and (road_type <= 7)  then
			if road_type_list[tonumber(road_type)] then
				--> urban
				return drive_mileage_point(app_name, 2, nil)
			elseif road_type == 0 then
				--> highway
				return drive_mileage_point(app_name, 10, nil)
			end
		end
		return false
	end
end

local valid_city = {
	["310000"] = true,	-->"上海"
	--[[
	["110000"] = true,	-->"北京"
	["120000"] = true,	-->"天津"
	["130100"] = true,	-->"石家庄"
	["140100"] = true,	-->"太原"
	["210100"] = true,	-->"沈阳"
	["210200"] = true,	-->"大连"
	["220100"] = true,	-->"长春"
	["320100"] = true,	-->"南京"
	["320200"] = true,	-->"无锡"
	["320400"] = true,	-->"常州"
	["320500"] = true,	-->"苏州"
	["330100"] = true,	-->"杭州"
	["330200"] = true,	-->"宁波"
	["330300"] = true,	-->"温州"
	["330400"] = true,	-->"嘉兴"
	["330600"] = true,	-->"绍兴"
	["330700"] = true,	-->"金华"
	["331000"] = true,	-->"台州"
	["340100"] = true,	-->"合肥"
	["350100"] = true,	-->"福州"
	["350200"] = true,	-->"厦门"
	["350500"] = true,	-->"泉州"
	["370100"] = true,	-->"济南"
	["370200"] = true,	-->"青岛"
	["420100"] = true,	-->"武汉"
	["430100"] = true,	-->"长沙"
	["440100"] = true,	-->"广州"
	["440300"] = true,	-->"深圳"
	["440400"] = true,	-->"珠海"
	["440600"] = true,	-->"佛山"
	["441300"] = true,	-->"惠州"
	["441900"] = true,	-->"东莞"
	["442000"] = true,	-->"中山"
	["500000"] = true,	-->"重庆"
	["510100"] = true,	-->"成都"
	["530100"] = true,	-->"昆明"
	["610100"] = true,	-->"西安"
	["630100"] = true,	-->"西宁"
	["650100"] = true,	-->"乌鲁木齐"
	--]]
}

function work()
	only.log("I", "l_f_road_traffic working ... ")
	--only.log("D", "l_f_road_traffic working ... ")
	local lon = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
	local lat = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
	local grid = string.format('%d&%d',tonumber(lon) * 100, tonumber(lat) * 100)
	--only.log("D", "redis grid key:" .. grid)

	local ok, code_jo = redis_api.cmd('mapGridOnePercent',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', grid)
	if not ok then
		only.log("I", "can't get code from redis!")
		return
	end
	--only.log("D", "json data:" .. code_jo)
	local ok, code_tab = pcall(cjson.decode, code_jo)
	if not ok then
		only.log("I", "can't decode code! " .. code_tab)
		return
	end

	--only.log('D', string.format("[code_tab: %s]", scan.dump(code_tab)))
	--[[
	--]]
	if not ( valid_city[ tostring(code_tab['countyCode']) ] or valid_city[ tostring(code_tab['cityCode']) ]
		or valid_city[ tostring(code_tab['provinceCode']) ] ) then
		only.log("I", "this city has no map data!!!")
		return
	end
	only.log("I", "l_f_road_traffic triggering ... ")
	local app_uri = "p2p_traffic_remind"
	local path = string.gsub(app_uri, "?.*", "")
	local app_srv = link["OWN_DIED"]["http"][ path ]
	local data = utils.compose_http_json_request(app_srv, app_uri, nil, APP_POOL["OUR_BODY_DATA"])
	http_api.http(app_srv, data, false)
end
