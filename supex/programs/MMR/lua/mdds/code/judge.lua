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
local weibo     = require("weibo")
local init_data = require("init_data")

--add by pengyangyang
local four_miles_ahead = require("four_miles_ahead")


module('judge', package.seeall)

--应用状态（开启/关闭）
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

--前方4公里
function is_4_miles_ahead_have_poi(app_name)
	return  four_miles_ahead.four_miles_ahead_logic()
end
