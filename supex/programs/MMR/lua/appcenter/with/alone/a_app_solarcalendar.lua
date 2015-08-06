--
-- 版权声明: 暂无
-- 文件名称: a_app_solarcalendar.lua
-- 创建者  : 
-- 创建日期: 
-- 文件描述: 本文件主要功能是给用户发送青年小阳历。
-- 历史记录: 无
--
local APP_POOL           = require("pool")
local APP_UTILS          = require("utils")
local APP_ONLY           = require("only")
local APP_CFG            = require("cfg")
local APP_CONFIG_LIST    = require("CONFIG_LIST")
local APP_REDIS_API      = require("redis_pool_api")
local APP_BOOL_FUNC_LIST = require("BOOL_FUNC_LIST")
local APP_WORK_FUNC_LIST = require("WORK_FUNC_LIST")
local link               = require("link")

module("a_app_solarcalendar", package.seeall)

function bind()
	return '{}'
end

function match()
	return true
end

local spx_txt_to_voice  = link["OWN_DIED"]["http"]["spx_txt_to_voice"]
local personalWeibo = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]

---- 青年小阳历
local solarCalendar = {
        appKey = '2469016736',
        secret = '18171DAE4F3BA8D7FC62E17CF1E607B8019DD6DB',
    }

local function txt_2_voice(text)	
	only.log('D',string.format("txt_2_voice: %s",text))
	return utils.txt_2_voice( spx_txt_to_voice, solarCalendar.appKey , solarCalendar.secret,text )
end


local split_str                = 'abc_appcenter_really_send_msg_to_weibo_test_2014_test_really'
local data_multi_personal_head = 'POST /weiboapi/v2/sendMultimediaPersonalWeibo HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\nContent-Type:content-type:multipart/form-data;boundary=' .. split_str .. '\r\n\r\n%s'
local data_splitend            = "--" .. split_str .. "--"
local data_format_normal       = '--' .. split_str .. '\r\nContent-Disposition:form-data;name="%s"\r\n\r\n%s\r\n'

---- 发送个人多媒体微博 
function really_send_multi_personal_weibo(host_name,host_port, wb )
    if not wb['appKey'] then
        wb['appKey'] = solarCalendar.appKey
        wb['sign'] = utils.gen_sign(wb,solarCalendar.secret)
    end
    local data_tab = {}
    for i,v in pairs(wb) do
        table.insert(data_tab,string.format(data_format_normal,i,v))
    end
    local data = string.format("%s%s",table.concat( data_tab, "") , data_splitend )
    local req = string.format(data_multi_personal_head,host_name,host_port,#data,data)
    return utils.really_send_multi_personal_weibo(host_name,host_port,req)
end


local function work_solar_calendar(accountID)
	---- 获取当前URL失败
	local cur_date = os.date("%Y%m%d")
	local file_url = string.format("http://127.0.0.1/productList/solarCalendar/%s.amr",cur_date)
	local exp_time = 15*60
	local tab = {
		level = 80,
		interval = exp_time,
		receiverAccountID = accountID,
		multimediaURL = tostring(file_url) ,
		senderType    = 2,   ---------添加发送类型区分微博来源 1:WEME    2:system    3:other
	}
	tab['appKey'] = solarCalendar.appKey
	tab['sign']   = utils.gen_sign(tab,solarCalendar.secret)

	local ok_status,ok_bizid = really_send_multi_personal_weibo( personalWeibo.host, personalWeibo.port, tab)
	if not ok_status or ok_bizid == nil then
		only.log('D',string.format('weibo send failed! %s, %s',accountID,file_url))
	else
		APP_REDIS_API.cmd('private',accountID, 'set', accountID ..  ':solarCalendar', cur_date)
		only.log('D',string.format('weibo send succed! %s, %s  bizid:%s ',accountID,file_url,ok_bizid))
		local time = os.time()
		local travelID  = nil 
		local appKey = solarCalendar.appKey
		local ok, imei =  APP_REDIS_API.cmd('private',accountID,'get', string.format("%s:IMEI", accountID))
		if ok and imei then
			ok, travelID = APP_REDIS_API.cmd('private',accountID, 'get', string.format("%s:travelID", imei))
		end 
		local ok_date, cur_date = APP_REDIS_API.cmd('private',accountID,'get', string.format("%s:speedDistribution", accountID))
		if not ok_date or not cur_date then
			cur_date = os.date("%Y%m")
			APP_REDIS_API.cmd('private',accountID, 'set', string.format("%s:speedDistribution", accountID), cur_date)
		end 
		--[[
		if ok and imei and accountID and travelID and cur_date then
			local datacore_statistics_var_key = accountID .. ":" .. travelID .. ":" .. appKey ..":" .. cur_date
			local datacore_statistics_var_value = bizid .. ":" .. time
			APP_REDIS_API.cmd('dataCoreRedis','sadd', datacore_statistics_var_key, datacore_statistics_var_value)
			APP_REDIS_API.cmd('dataCoreRedis','expire', datacore_statistics_var_key, 48*3600)
		end 
		--]]
	end
end

function work()
	APP_ONLY.log("I", "a_app_solarcalendar working ... ")
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	work_solar_calendar(accountID)
end

