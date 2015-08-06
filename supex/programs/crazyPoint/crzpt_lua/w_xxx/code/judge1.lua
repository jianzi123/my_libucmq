-- auth: baoxue
-- time: 2014.04.27

    local redis_api = require('redis_pool_api')
    local APP_CONFIG_LIST = require('CONFIG_LIST1')
    local APP_POOL = require('pool1')
    local cjson = require('cjson')
    --local link  = require('link1')
    local cutils = require('cutils')
    local supex = require('supex1')
    local utils = require('utils')
    local only = require('only')
    local http_short = require('http_short_api')
    local weibo              = require("weibo1")
    local init_data = require("init_data1")


    module('judge1', package.seeall)


    function freq_filter( app_name, accountid )
        local cause = APP_CONFIG_LIST["OWN_LIST"][app_name]["ways"]["cause"]
        local class = cause["trigger_type"]
        local num = cause["fix_num"]
        local delay = cause["delay"]
        local accountID = accountid
        --> func list
        local check_list = {
            every_time = function( ... )
                return true
            end,
            once_life = function( ... )
                local keyct = string.format("%s:onceAllLife", accountID)
                local ok,val = redis_api.cmd("private",accountID, "sismember", keyct, app_name)
			if not ok then return false end
			if not val then
				redis_api.cmd("private",accountID, "sadd", keyct, app_name)
				return true
			else
				return false
			end
		end,
		power_on = function( ... )
			local keyct = string.format("%s:oncePowerOn", accountID)
			local ok,val = redis_api.cmd("private",accountID, "sismember", keyct, app_name)
			if not ok then return false end
			if not val then
				redis_api.cmd("private",accountID, "sadd", keyct, app_name)
				return true
			else
				return false
			end
		end,
		one_day = function( ... )
			local keyct = string.format("%s:%s:everyDay", accountID, app_name)
			local ok,val = redis_api.cmd("private",accountID, "get", keyct)
			if not ok then return false end
			if not val then
				local over = 86400 - ((os.time() + 28800)%(86400))
				redis_api.cmd("private",accountID, "set", keyct, 1)
				redis_api.cmd("private",accountID, "expire", keyct, over)
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
				redis_api.cmd("private",accountID, "incr", keyct)
			end
			return true
		end,
		fixed_time = function( ... )
			local keyct = string.format("%s:%s:fixedInterval", accountID, app_name)
			local ok,val = redis_api.cmd("private",accountID, "get", keyct)
			if not ok then return false end
			if not val then
				redis_api.cmd("private",accountID, "set", keyct, 1)
				redis_api.cmd("private",accountID, "expire", keyct, delay)
			else
				if tonumber(val) >= num then return false end
				redis_api.cmd("private",accountID, "incr", keyct)
			end
			return true
		end,
	}
	return check_list[ class ]( )
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
			local ok,val = redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "srem", keyct, app_name)
			if not ok then return false end
			return true
		end,
		power_on = function( ... )
			local keyct = string.format("%s:oncePowerOn", uid)
			local ok,val = redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "srem", keyct, app_name)
			if not ok then return false end
			return true
		end,
		one_day = function( ... )
			local keyct = string.format("%s:%s:everyDay", uid, app_name)
			local ok,val = redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", keyct)
			if not ok then return false end
			if val and (tonumber(val) > 0) then
				redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "decr", keyct)
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
			local ok,val = redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", keyct)
			if not ok then return false end
			if val and (tonumber(val) > 0) then
				redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "decr", keyct)
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
	local ok, ret = init_data.lru_hash_cache_get("private",accountID, weather_forcast_key)
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
		init_data.lru_hash_cache_set("private",accountID,weather_forcast_key, '1', 1)
		return true
	end
	return false
end

--名  称：get_city_code
--功  能：通过经纬度获取城市到代码
--参  数：accountID
--返回值：城市代码
function get_city_code(accountID)
    local ok,gps_str = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',accountID .. ":currentBL")
    if ok and gps_str then
        local gps_info = utils.str_split(gps_str,",")
        local longitude = gps_info[1]
        local latitude  = gps_info[2]
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
end

function user_control(app_name,accountID)
     local accountID = accountID
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

