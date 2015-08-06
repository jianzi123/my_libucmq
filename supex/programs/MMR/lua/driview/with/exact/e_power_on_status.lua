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
local weibo = require('weibo')


module('e_power_on_status', package.seeall)

function bind()
	return '["powerOn", "accountID", "tokenCode", "model"]'
end


local function get_tab_to_json()
	local tab = {
		accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"],
		powerOn   = tostring(APP_POOL["OUR_BODY_TABLE"]["powerOn"]),
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


local function get_tab_to_kv( )
	local tab = {
		accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"],
		powerOn   = tostring(APP_POOL["OUR_BODY_TABLE"]["powerOn"]),
		tokenCode = APP_POOL["OUR_BODY_TABLE"]["tokenCode"],
		model     = APP_POOL["OUR_BODY_TABLE"]["model"],
	}
	return utils.table_to_kv(tab)
end


local function get_tab_to_kv_app( )
	local tab = {
		accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"],
		powerOn   = tostring(APP_POOL["OUR_BODY_TABLE"]["powerOn"]),
		model     = APP_POOL["OUR_BODY_TABLE"]["model"],
	}

	local imei = nil
	if #tab.accountID == 15 then
		imei = tab.accountID
	else
		local ok_status,ok_val = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',tab.accountID .. ":IMEI")
		if not ok_status or ok_val == nil then
			only.log('D',string.format("get accountID:IMEI failed,accountID:%s",tab.accountID))
			return nil
		end
		imei = ok_val
	end

	if imei then
		local ok_status ,ok_val = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', imei .. ":travelID")
		if not ok_status or ok_val == nil then
			only.log('D',string.format("get IMEI:travelID failed,accountID:%s IMEI:%s ",tab.accountID,imei))
			return nil
		end
		tab['travelID'] = ok_val
	end

	return utils.table_to_kv(tab)
end



---- 判断当前是否已经发送 
function check_solarcalendar_is_send(body_data)
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local cur_date = os.date("%Y%m%d")
	local str_key = string.format("%s:solarCalendar",accountID)
	if not weibo.check_driview_subscribed_msg(accountID, weibo.DRI_APP_SOLARCALENDER) then
		only.log('D','青年小阳历，被客户禁止!')
		return false
	end

	only.log('D',string.format('redis key is :%s',str_key))

	local ok_status,ok_cur = redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',str_key)
	if not ok_status then 
		only.log('E','connect redis failed!')
		return false 
	end
	if ok_cur and tostring(ok_cur) == tostring(cur_date) then
		only.log('D',string.format("%s %s Already send solar Calendar",accountID, cur_date ))
		return false
	end
	only.log('D','====ok_really_work work_to_solarcalendar()===>===')
	work_to_solarcalendar(body_data)
end


---- 青年小阳-- 2014-05-15
function work_to_solarcalendar(body_data)
	local app_server = link["OWN_DIED"]["http"]["appcenterApply.json"]
	local data = utils.compose_http_json_request(app_server, "appcenterApply.json?app_name=a_app_solarcalendar", nil, body_data)
	local ok_status = http_short.http(app_server, data, false)
	if not ok_status then
		only.log('D',string.format("post data %s:%s failed!",app_server.host,app_server.port))
	end
end


---- 用户上线提示
function work_to_poweron_online(body_data)
	local app_server = link["OWN_DIED"]["http"]["appcenterApply.json"]
	local data = utils.compose_http_json_request(app_server, "appcenterApply.json?app_name=a_app_report_user_online", nil, body_data)
	local ok_status = http_short.http(app_server, data, false)
	if not ok_status then
		only.log('D',string.format("post data %s:%s failed!",app_server.host,app_server.port))
	end
end


--3 用户开机,提示3句话日志
function work_to_poweron_standardapp(body_data)
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	if not accountID or #accountID == 15 then 
		----没有绑定accountID的终端,不触发
		only.log('D',string.format("accountID:%s is imei ,jmp work_to_poweron_standardapp-->--",accountID))
		return 
	end

	----- 没有订阅----
	if not weibo.check_thirdapp_subscribed_msg(accountID,'1209071138') then
		only.log('D',string.format("accountID:%s is imei ,not subscribed  work_to_poweron_standardapp-->--",accountID))
		return false
	end

	local ok_status,ok_ret = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':newbieGuide')
	if (tonumber(ok_ret) or 0 ) ~= 1 then
		only.log('D',string.format("accountID:%s newbieGuide != 1 jmp send single",accountID))
		return false
	end

	local app_server = link["OWN_DIED"]["http"]["customizationapp/poweronDiaryStart"]
	local data = utils.post_data("customizationapp/poweronDiaryStart", app_server, body_data)
	local ok_status = http_short.http(app_server, data, false)
	if not ok_status then
		only.log('D',string.format("post data %s:%s failed!",app_server.host,app_server.port))
		return false
	end
	only.log('D',data)
end


function work_to_poweron_get_sharevalue()

	only.log('D',"=============work_to_poweron_get_sharevalue============")

	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	if not accountID or #accountID ~= 10 then
		only.log('E',string.format("accountID: %s  not normal accountID, have not share value ", accountID ))
		return false
	end

	local app_server = link["OWN_DIED"]["http"]["crazyapi/v2/shareValueTask"]

	-- taskID = 开机 
	local urlpath = "/crazyapi/v2/shareValueTask?jobId=SUPEX_DRIVIEW_POWERON&parameter=taskType%3D1%26taskID%3D1%26accountID%3D" .. accountID
	local data_format =  'GET %s HTTP/1.0\r\n' ..
	'HOST:%s:%s\r\n\r\n'

	local data = string.format(data_format,urlpath,app_server.host,app_server.port)
	local ok_status = http_short.http(app_server, data, false)
	if not ok_status then
		only.log('D',string.format("post data %s:%s failed!",app_server.host,app_server.port))
		return false
	end
	return true

end

function match()
	return true
end


function work()

	only.log('D','==== enter e_power_on_status===========')

	local body_data_json = get_tab_to_json()

	if body_data_json then
		--1 青年小阳历
		check_solarcalendar_is_send(body_data_json)
		--2 用户开机提醒
		--work_to_poweron_online(body_data_json)
	end

	local body_data_kv = get_tab_to_kv_app()
	if body_data_kv then
		--3 用户开机,提示3句话日志
		work_to_poweron_standardapp(body_data_kv)
	end

	-- 用户谢尔成长值/开机通知
	--work_to_poweron_get_sharevalue()

	only.log('D','==== leave e_power_on_status============')
end

