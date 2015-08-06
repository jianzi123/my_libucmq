local APP_POOL = require("pool1")
--local APP_UTILS = require("utils")
local APP_ONLY = require("only")
--local APP_REDIS_API = require("redis_pool_api")
local APP_CFG = require("cfg1")
local APP_CONFIG_LIST = require("CONFIG_LIST1")
--local APP_BOOL_FUNC_LIST = require("BOOL_FUNC_LIST")
local APP_WORK_FUNC_LIST = require("WORK_FUNC_LIST1")
local APP_JUDGE = require("judge1")
--local APP_MAP = require("map")


module("l_f_weather_forcast", package.seeall)


--[[
function match()
	if not (APP_JUDGE.user_control('l_f_weather_forcast')) then
		return false 
	end
	if not (APP_JUDGE.is_weather_forcast("l_f_weather_forcast")) then
		return false
	end
	if not (APP_POOL["OUR_BODY_TABLE"]["collect"] == true) then
		return false
	end
	return true
end
]]--
function work(accountID,info_table)
	if not (APP_JUDGE.user_control('l_f_weather_forcast',accountID)) then
		return false 
	end
    --[[
	if not (info_table["collect"] == true) then
		return false
	end
    ]]--
	 APP_WORK_FUNC_LIST["app_task_forward"]( "l_f_weather_forcast" ,accountID,info_table)
end

