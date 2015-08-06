-- auth: zhangjl
-- time: 2014.01.26

local only          = require('only')
local cjson         = require 'cjson'
local APP_POOL      = require('pool')
local utils         = require('utils')
local redis_api     = require('redis_pool_api')
local http_short    = require('http_short_api')
local link          = require('link')
local init_data     = require("init_data")
local judge         = require("judge")
local connect	    = require("crzptY_client")
module('e_power_on', package.seeall)

VOICE_COMMAND_NOTEPAD   = 1
VOICE_COMMAND_CHANNEL   = 2
VOICE_COMMAND_SINAWEIBO = 3 

function bind()
	return '["powerOn", "accountID", "tokenCode", "model"]'
end

function match()
	return true
end
--临时频道已经下线，不再使用
local function power_on_set_channel(accountid)
	local default_channel = "10086"
	local VOICE_COMMAND_CHANNEL = 2
        --[[
	local ok_status,ok_tmp_channel = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',
				accountid .. ':tempChannel:groupVoice')
	if ok_status and ok_tmp_channel and #tostring(ok_tmp_channel) > 2 then
		---- ++按键优先临时频道
		local tmp_channle_key = ok_tmp_channel .. ':channelOnlineUser'
		redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"],'sadd', tmp_channle_key, accountid)
	else
        ]]--
	local ok_status,ok_channel = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',
				accountid .. ':currentChannel:groupVoice')
        if ok_status then
			ok_channel = ok_channel or default_channel
			local tmp_channle_key = ok_channel .. ':channelOnlineUser'
			redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"],'sadd', tmp_channle_key, accountid)
	end
	--end

	local ok_status,ok_voice_type = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',
							accountid .. ':voiceCommandCustomType')
	if ok_status and ok_voice_type then
		if tonumber(ok_voice_type) == VOICE_COMMAND_CHANNEL then
			local ok_status,ok_channel = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',
							accountid .. ':currentChannel:voiceCommand')
			if ok_status then
				ok_channel = ok_channel or default_channel
				local tmp_channle_key = ok_channel .. ':channelOnlineUser'
				redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"],'sadd', 
							tmp_channle_key, accountid)
			end
		end
	end
end

-- local function power_on_set_channel_weme(accountid)
-- 	local default_channel = "10086"
-- 	local cur_accountid = APP_POOL["OUR_BODY_TABLE"]["accountID"]
-- 	local ok_status,ok_channel = redis_api.cmd('private', cur_accountid ,'get', accountid .. ':currentChannel:groupVoice')
-- 	if ok_status then
-- 		ok_channel = ok_channel or get_default_channel_idx(cur_accountid,default_channel)
-- 		if ok_channel then
-- 			redis_api.cmd('statistic', cur_accountid ,'sadd', ok_channel .. ':channelOnlineUser', accountid)
-- 		end
-- 	end
-- 	local ok_status,ok_channel = redis_api.cmd('private', cur_accountid ,'get', accountid .. ':currentChannel:voiceCommand')
-- 	if ok_status and ok_channel then
-- 		---- + 按键未定义,
-- 		redis_api.cmd('statistic',cur_accountid,'sadd',  ok_channel .. ':channelOnlineUser', accountid)
-- 	end
-- end

--
--名  称：power_on_reset
--功  能：每次开机对redis的初始化
--参  数：api_data
--返回值：无
--

--
--名  称：power_on_reset
--功  能：每次开机对redis的初始化
--参  数：api_data
--返回值：无
--

--
--名  称：power_on_reset
--功  能：每次开机对redis的初始化
--参  数：api_data
--返回值：无
--

local function power_on_reset(api_data)
    -- clean key
  	local accountID = api_data["accountID"]
	local cur_date = os.date("%Y%m")
    --keyName:powerOffTimestamp          place:private  des:初始化关机时间戳      action:w
    --keyName:currentSpeed               place:private  des:初始化速度            action:w
    --keyName:currentDirection           place:private  des:初始化方向角          action:w
    --keyName:heartbeatTimestamp         place:private  des:获取心跳时间戳        action:w
    --keyName:speedDistribution          place:private  des:速度分布              action:w
    --keyName:continuousDrivingKeepData  place:private  des:连续驾驶保留数据      action:w
    --keyName:lowSpeedMileages           place:private  des:低速里程              action:w
    --keyName:overSpeedMileages          place:private  des:超速里程              action:w
    --keyName:driviewPowerOn             place:private  des:driview模块初始化为1  action:w

  	redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'mset',
                          	accountID.. ':currentSpeed', 0, 
                          	accountID.. ':currentDirection', 0,
				accountID.. ':speedDistribution', cur_date)
				--accountID.. ':continuousDrivingKeepData', os.time() .. ':0',
				--accountID.. ':driviewPowerOn',1)
        
    --keyName:currentMileage    place:priviate_hash  des:当前里程       action:w
    --keyName:currenOnlineTime  place:priviate_hash  des:当前在线时间   action:w
	

    --keyName:overSpeedStartime     place:driview  des:记录上次超速时间      action:w
    --keyName:overSpeedPointCount   place:driview  des:记录限速值和状态标识  action:w
    --keyName:patternStopLastTime   place:driview  des:模式停止最后时间      action:w
    --keyName:isOverSpeedPointCount place:driview  des:记录超速点            action:w
  	redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"],'mset',accountID.. ':isOverSpeedPointCount',-1)
	--keyName:onlineUser:set  place:statistic  des:存放在线用户  action:w
	redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd','onlineUser:set', accountID)
	
	--keyName:onceStepKeySet   place:private  des:判断按键设置  action:r
	local ok, keys = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "smembers", accountID .. ":onceStepKeysSet")
	if ok then
		for i=1,#(keys or {}) do
			redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", keys[i])
		end
		redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", 
				accountID .. ":onceStepKeysSet")
	end
	
	
	local key  = accountID .. ':configTimestamp'
	local value = tostring(api_data["timestamp"])
	local value_len = #value
    --keyName:configTimestamp\timestamp  place:private本地缓存  des:存放用户开机时间 action:r
	init_data.lru_hash_cache_set("driview",accountID,key, value, value_len)

	local key  = accountID .. ':userMsgSubscribed'
	local ok,value= redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', key)
	if ok and value then
		local value_len = #value
		--得到订阅信息放到本地缓存
		local ok, ret = init_data.lru_hash_cache_set("private",accountID,key, value, value_len)
	end
        
        --keyName:roadIDPOIType   place:driview  des:匹配道路id到兴趣点类型是否在集合中     action:r
        local ok, keys = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "smembers", 
				accountID .. ":roadIDPOIType")
	if ok then
        --删除道路id兴趣点
		for i=1,#(keys or {}) do
			redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", accountID..":" ..keys[i])
                        only.log('D',"delete fetch 4 miles key :" .. accountID .. ":"..keys[i])
		end
			redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", accountID .. ":roadIDPOIType")
	end
        
    --keyName:oncePowerOn            place:private       des:删除开机记录      action:w
    --keyName:pattern                place:private       des:模式              action:w
    --keyName:couponIDSet            place:private       des:                  action:w
    --keyName:continuousDrivingRepeatFilter     place:private  des:连续驾驶重复过滤器  action:w
    --keyName:POIID                  place:private_hash  des:兴趣点ID          action:w
    --keyName:thrityPoints           place:driview       des:三十点(不确定，根据英文翻译) action:w
    --keyName:overSpeedHash          place:driview       des:超速哈希          action:w
    --keyName:roadSafetyCamera       place:driview       des:道路安全摄像头    action:w
    --keyName:lastLimitSpeed         place:driview       des:最后限速          action:w
    --keyName:lastPassedRoadRootID   place:driview       des:最后经过到roadID  action:w
    --keyName:isOverSpeedStartTime   place:driview       des:上次超速时间      action:w
	redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del",accountID .. ':oncePowerOn')
       
	--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', accountID .. ':pattern')
	--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', accountID .. ':couponIDSet')
	--redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', accountID .. ':continuousDrivingRepeatFilter')
	
	redis_api.cmd('private_hash',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', accountID .. ':POIID')
	redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', accountID .. ':roadSafetyCamera')
	redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', accountID .. ':isOverSpeedStartTime', 0)
        --连续驾驶使用
        redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"],'del', 
				APP_POOL["OUR_BODY_TABLE"]["accountID"]..':sumcontinuousDrivingRepeatFilter')
        redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"],'del',
				APP_POOL["OUR_BODY_TABLE"]["accountID"]..':actualcontinuousDrivingRepeatFilter')
        --keyName: weatherForecast  place:private本地缓存  des:是否播报天气到标识  action:w
	init_data.lru_hash_cache_set("driview",accountID,accountID .. ':weatherForecast', 0, 1)
        local init_city_code_value = "err"
        init_data.lru_hash_cache_set("driview",accountID,accountID .. ':cityCode', init_city_code_value, #init_city_code_value)
        
        local ok_status, travelID = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:travelID", init_data_imei ))
        if ok_status and travelID then
                app_lua_lru_cache_set_value(accountID..':travelID', tostring(travelID), #(tostring(travelID)))
        end
	-- 开机设置频道的参数  2014-07-25
	--power_on_set_channel(accountID)
end


function work()

	local api_data = {
		accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"],
		tokenCode = APP_POOL["OUR_BODY_TABLE"]["tokenCode"],
		timestamp = os.time(),
	}
    local table_data = APP_POOL["OUR_BODY_DATA"]
	connect.connect(table_data)
	power_on_reset(api_data)
	only.log('D', '=============e_power_on[' .. api_data["accountID"].. ']==============')
end
