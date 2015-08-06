--
-- 版权声明: 暂无
-- 文件名称: a_app_report_user_offline.lua
-- 创建者  :  
-- 创建日期: 
-- 文件描述:本文件主要功能是当用户关机的时候给该用户4公里内的在线用户发送关机微博，当4公里内的在线用户数超过50时会随机选择50以内的用户发送。
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
local alone_utils        = require('alone_utils')

module("a_app_report_user_offline", package.seeall)


local appInfo = {
	appKey = '3202518884',
	secret = 'CACB1026D77B65CB3986D42C938130A75CCA852A',
}

local max_user = 50
local distance = 4000

function bind()
	return '{}'
end

function match()
	return true
end

local function work_offline(accountID)
	local icount = alone_utils.get_onlineuser_count()
	local ok_status,account_tab = nil,nil
	if icount > max_user then
		local ok_point,longitude ,latitude = alone_utils.get_point(accountID)
		if ok_point and tonumber(longitude) ~= 0 and tonumber(latitude) ~= 0  then
			ok_status , account_tab = alone_utils.get_around_daoke(longitude, latitude, distance)
		end
	end

	local random_tab = {}
	if not account_tab or #account_tab < max_user then 
		random_tab = alone_utils.get_onlineuser_by_random(max_user)
	end

	if not account_tab then 
		account_tab = random_tab 
	else
		for k,v in pairs(random_tab) do
			account_tab[v] = v
		end
	end

	if not account_tab then 
		only.log('D',string.format("account_tab is nil "))
		return 
	end

	local ok_status,ok_nickname = alone_utils.get_nickname(accountID)
	if #tostring(accountID) == 10 and ok_nickname == nil then
		local ok_status,ok_imei = alone_utils.get_imei_by_accountid(accountID)
		if ok_status and ok_imei then
			ok_nickname = alone_utils.get_daokenum_by_imei(ok_imei)
		end
	elseif #tostring(accountID) == 15 then
		ok_nickname = alone_utils.get_daokenum_by_imei(accountID)
	end

	local text = string.format("道客[%s]，关机了。",ok_nickname)
	local  ok_status,ok_url,ok_fileid = alone_utils.txt_2_voice(appInfo.appKey,appInfo.secret,text)
	if not ok_status or ok_url == nil then 
		only.log('D',string.format("accountID:%s text;%s  txt to voice failed!",accountID,text))
		return 
	end

	for i,v in pairs(account_tab) do
		if v ~= accountID then
			local tab = {
				appKey            = appInfo.appKey,
				level             = 80,
				interval          = 15,
				senderAccountID   = accountID,
				receiverAccountID = v,
				sourceID          = ok_fileid,
				content           = text,
				multimediaURL     = tostring(ok_url) ,
				senderType        = 2,   ---------添加发送类型区分微博来源 1:WEME    2:system    3:other
			}
			tab['sign'] = utils.gen_sign(tab, appInfo.secret)
			local ok,bizid = alone_utils.really_send_multi_personal_weibo(tab)
			if ok then
				only.log('D',string.format("accountid:%s  to:%s  bizid: %s ", accountID, v , bizid ))
				local time = os.time()
				local travelID  = nil 
				local appKey = appInfo.appKey
				local ok, imei =  APP_REDIS_API.cmd('private',accountID,'get', string.format("%s:IMEI", accountID))
				if ok and imei then
					ok, travelID = APP_REDIS_API.cmd('private',accountID,'get', string.format("%s:travelID", imei))
				end 
				local ok_date, cur_date = APP_REDIS_API.cmd('private',accountID,'get', string.format("%s:speedDistribution", accountID))
				if not ok_date or not cur_date then
					cur_date = os.date("%Y%m")
					APP_REDIS_API.cmd('private',accountID,'set', string.format("%s:speedDistribution", accountID), cur_date)
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
	end
end


function work()
	APP_ONLY.log("I", "a_app_report_user_offline ... ")
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	work_offline(accountID)
end

