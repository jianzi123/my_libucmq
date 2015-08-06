-- auth: baoxue
-- time: 2014.04.27

local utils = require('utils')
local cutils = require('cutils')
local only = require('only')
local APP_POOL = require('pool')
local redis_api = require('redis_pool_api')
local mysql_api = require('mysql_pool_api')
local http_api = require('http_short_api')
local cjson = require('cjson')
local link = require('link')
local init_data = require("init_data")
local judge =require("judge")



module('l_collect_gps', package.seeall)

function bind()
	return '["collect", "accountID", "GPSTime", "direction", "speed"]'
end

function match()
	return true
end

VOICE_COMMAND_NOTEPAD   = 1
VOICE_COMMAND_CHANNEL   = 2
VOICE_COMMAND_SINAWEIBO = 3 




function update_citycode_channel(accountID)
        local current_online_time = init_data.init_current_online_time
        --每５分钟获取下城市
	local remainder = current_online_time % 300
	if (remainder >= 0) and (remainder < 20) then
                local ok,cityCode = judge.get_city_code()
                if ok and cityCode then
                        redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"],'sadd', cityCode .. ':cityOnlineUser', accountID)
                        init_data.lru_hash_cache_set('private',accountID, accountID..':cityCode', tostring(cityCode), #(tostring(cityCode)))
                end
        end
end


function statistics_speed_distribution( speed, accountID, imei )
    	cur_data = os.date("%Y%m%d",os.time())
	for k,v in ipairs( speed ) do
	    if v ~= nil and v <= 150 then
		key = string.format("%02d",math.ceil(v/10))	    
		local ok_status, count = redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'hincrby',accountID .. ':' .. imei .. ':speedDistribution:' .. cur_data, key, 1 )
	    elseif v > 150 then
		local ok_status, count = redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'hincrby',accountID .. ':' .. imei .. ':speedDistribution:' .. cur_data, 16, 1 )
	    end
	end
	only.log("D","statistics_speed_distribution function end !")
end

function work()
    	
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local GPSTime = APP_POOL["OUR_BODY_TABLE"]["GPSTime"]
	local direction = APP_POOL["OUR_BODY_TABLE"]["direction"]
	local speed = APP_POOL["OUR_BODY_TABLE"]["speed"]
	local altitude = APP_POOL["OUR_BODY_TABLE"]["altitude"]
        local latitude = APP_POOL["OUR_BODY_TABLE"]["latitude"]
	local longitude = APP_POOL["OUR_BODY_TABLE"]["longitude"]
        
        update_citycode_channel(accountID)
        init_data.speed_statistics()
        
  	
        local gps_info = longitude[1]..","..latitude[1]
        redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'mset',accountID..':currentBL',gps_info,accountID..':currentGPSTime',GPSTime[1],accountID..':currentSpeed',speed[1],accountID..':currentDirection',direction[1],accountID..':currentAltitude',altitude[1])
end
