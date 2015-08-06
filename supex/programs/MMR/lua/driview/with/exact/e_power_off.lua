-- auth: zhangjl
-- time: 2014.01.11

local only = require('only')
local APP_POOL = require('pool')
local redis_api = require('redis_pool_api')
local cjson = require('cjson')
local utils = require('utils')
local judge = require('judge')


module('e_power_off', package.seeall)

function bind()
	return '["powerOff", "accountID", "tokenCode", "model"]'
end

function match()
	return true
end


local function power_off_reset(api_data)
        --[[
	redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', api_data["accountID"] .. ':powerOffTimestamp', api_data["timestamp"])
	-- remove the user in fleet
	local ok, user_fleet_tab = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'smembers', api_data["accountID"] .. ':carFleetSet')
	if #user_fleet_tab ~= 0 then
		redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'srem', user_fleet_tab[1] .. ':fleetOnlineUser', api_data["accountID"])
	end
        ]]--
	local ok, last_config_time = redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', api_data["accountID"] .. ':configTimestamp')
	if ok and last_config_time then
		only.log('D', 'last_config_time:' .. last_config_time)
	end

	local ok, last_newstatus_time = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', api_data["accountID"] .. ':heartbeatTimestamp') 
	if ok and last_newstatus_time then
		only.log('D', 'last_newstatus_time:' .. last_newstatus_time)
	end
	-- this statistic info only for account?
	local current_online_time = 0
	if last_newstatus_time and (last_newstatus_time ~= 0) then
		current_online_time = api_data["timestamp"] - tonumber( last_config_time or 0 )
		only.log('D', 'current_online_time:' .. current_online_time)

		-- the time length after last newstatus
		--local time = api_data["timestamp"] - tonumber(last_newstatus_time)

		--redis_api.cmd('private_hash',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'incrby', api_data["accountID"] .. ':currentOnlineTime', time)
		--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'incrby', api_data["accountID"] .. ':totalOnlineTime', time)
	end

	redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'incrby', 'totalOnlineTime:count', current_online_time)
	redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'srem', 'onlineUser:set', api_data["accountID"])


	---- 	关机清理频道列表,移至feedbackapi  2014-07-07
	-- local default_channel = "12345"
	-- -------- 清理普通频道
	-- local ok,channel = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', api_data["accountID"] .. ':currentChannel:groupVoice')
	-- if not ok or not channel then
	-- 	channel = default_channel
	-- end
	-- redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'srem', channel .. ':channelOnlineUser', api_data["accountID"])

	-- -------- 清理语音命令对应的频道
	-- local ok,channel = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', api_data["accountID"] .. ':currentChannel:voiceCommand')
	-- if not ok or not channel then
	-- 	channel = default_channel
	-- end
	-- redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'srem', channel .. ':channelOnlineUser', api_data["accountID"])
	---- 关机清理频道列表,移至feedbackapi  2014-07-07



	-->>city online
	local ok, cityCode = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', api_data["accountID"] .. ":cityCode")
	if ok and cityCode then
		redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'srem', cityCode .. ':cityOnlineUser', api_data["accountID"])
	end
end

local function is_at_home(app_name)
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]

	----FIXED by jiang z.s. 2014-07-02
	local ok,gps_str = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ":currentBL")
	if not ok or not gps_str then
		return false 
	end
	local gps_info = utils.str_split(gps_str,",")
	lon = gps_info[1]
	lat  = gps_info[2]

        local ok, home_city = init_data.lru_hash_cache_get('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], accountID .. ':homeCityCode')
	if not ok or not home_city then
		only.log("I", "can't get homeCityCode from redis!")
		return false
	end

	local grid = string.format('%d&%d', tonumber(lon) * 100, tonumber(lat) * 100)
	only.log("D", "redis grid key:" .. grid)
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
	local city_code = code_tab['cityCode']
	if not city_code then
		only.log("E", "can't get cityCode from json!")
		return false
	end
	only.log("D", string.format("city_code:[%s] home_city:[%s]", city_code, home_city))
	if tonumber(city_code) ~= tonumber(home_city) then
		only.log('D', 'cityCode homeCityCode is not same!')
		return false
	end

	return true
end

function work()
	local api_data = {
		accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"],
		timestamp = os.time(),
	}
	-->> clean data in Redis
	power_off_reset(api_data)

	if is_at_home( "e_power_off" ) then
		-->> offsite key
		local ok, keys = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "smembers", api_data["accountID"] .. ":offsiteAllKeysSet")
		if ok then
			for i=1,#(keys or {}) do
				redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", keys[i])
			end
			redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", api_data["accountID"] .. ":offsiteAllKeysSet")
		end
	end

	only.log('D', string.format("=============e_power_off[%s]==============", api_data["accountID"]))
end

