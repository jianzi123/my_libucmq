-- auth: jiang z.s. 
-- time: 2014.05.19

local utils      = require('utils')
local only       = require('only')
local APP_POOL   = require('pool')
local cjson      = require('cjson')
local link       = require('link')
local mysql_api  = require("mysql_pool_api")
local redis_api  = require("redis_pool_api")
local http_short = require('http_short_api')


module('e_power_off_status', package.seeall)

function bind()
	return '["powerOff", "accountID", "tokenCode", "model"]'
end


local function get_tab_to_str(  )
	local tab = {
		accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"],
		powerOff   = tostring(APP_POOL["OUR_BODY_TABLE"]["powerOff"]),
		tokenCode = APP_POOL["OUR_BODY_TABLE"]["tokenCode"],
		model     = APP_POOL["OUR_BODY_TABLE"]["model"],
	}
	local ok_status,ok_ret = pcall(cjson.encode,tab)
	if not ok_status then
		only.log('E','cjson.encode failed!')
		return nil
	end
	return ok_ret
end

---- 用户关机提示
function work_to_poweroff_offline(body_data)
	local appcenter_server = link["OWN_DIED"]["http"]["appcenterApply.json"]
	local data = utils.compose_http_json_request(appcenter_server, "appcenterApply.json?app_name=a_app_report_user_offline", nil, body_data)
	http_short.http(appcenter_server, data, false)
end



function match()
	return true
end


function work()

	only.log('D','==== enter e_power_off_status===========')
	
	local body_data = get_tab_to_str()
	if body_data then
	--	work_to_poweroff_offline(body_data)
	end
    only.log('D','==== leave e_power_off_status============')
end

