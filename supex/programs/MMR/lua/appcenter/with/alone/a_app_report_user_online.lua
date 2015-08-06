--
-- 版权声明: 暂无
-- 文件名称: a_app_report_user_online.lua
-- 创建者  :  
-- 创建日期: 
-- 文件描述: 本文件主要功能是当用户开机的时候给与该用户同频道的用户发送开机微博，告诉他们某某用户上线了。
-- 历史记录: 无
--
local APP_POOL           = require("pool")
local APP_UTILS          = require("utils")
local APP_ONLY           = require("only")
local APP_CFG            = require("cfg")
local APP_CONFIG_LIST    = require("CONFIG_LIST")
local APP_REDIS_API      = require("redis_pool_api")
local redis_api      = require("redis_pool_api")
local APP_BOOL_FUNC_LIST = require("BOOL_FUNC_LIST")
local APP_WORK_FUNC_LIST = require("WORK_FUNC_LIST")
local link               = require("link")
local alone_utils        = require('alone_utils')
local weibo              = require("weibo")


module("a_app_report_user_online", package.seeall)


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
	APP_ONLY.log('D',"============a_app_report_user_online ==== match====")
	return true
end


VOICE_COMMAND_NOTEPAD   = 1
VOICE_COMMAND_CHANNEL   = 2
VOICE_COMMAND_SINAWEIBO = 3 


local function work_online(accountID)

	local ok_status 
	local account_tab = {}
	local channelSet = {}

	local cur_accountid = APP_POOL["OUR_BODY_TABLE"]["accountID"]

	local ok,channel = redis_api.cmd('private', cur_accountid , 'get', accountID .. ':currentChannel:groupVoice')
	if  ok and channel then
		local ok_ret, channel_set = redis_api.cmd("statistic",APP_POOL["OUR_BODY_TABLE"]["accountID"], "smembers", channel .. ':channelOnlineUser')
		if ok_ret and channel_set then
			channelSet[channel] = channel
			for k,v in pairs(channel_set)  do
				account_tab[v] = {}
				account_tab[v]['accountID'] = v
				account_tab[v]['channel'] = channel
			end
		end
	end

	local ok,channel = redis_api.cmd('private', cur_accountid , 'get', accountID .. ':currentChannel:voiceCommand')
	if  ok and channel then
		local ok_ret, channel_set = redis_api.cmd("statistic",APP_POOL["OUR_BODY_TABLE"]["accountID"], "smembers", channel .. ':channelOnlineUser')
		if ok_ret  and channel_set then
			channelSet[channel] = channel
			for i , v in pairs(channel_set) do
				if not account_tab[v]  then
					account_tab[v] = {}
					account_tab[v]['accountID'] = v
					account_tab[v]['channel'] = channel
				end
			end
		end
	end

	if account_tab[accountID] then
		account_tab[accountID] = nil;
	end

	if not next(account_tab) then 
		APP_ONLY.log('D',string.format("account_tab is nil "))
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

	---- 没有nickname,nickname为空,nickname包含字母,使用imei最后4位
	if not ok_nickname or #ok_nickname < 1 or string.match(ok_nickname,"%w+") then
		ok_nickname = nil
		local ok_status,ok_imei = alone_utils.get_imei_by_accountid(accountID)
		if ok_status and ok_imei then
			ok_nickname = alone_utils.get_daokenum_by_imei(ok_imei)
		end
	end

	if not ok_nickname or #ok_nickname < 1 then
		return
	end

	--local channel_name ='一二三四五' -- alone -- return number_to_chinese( string.sub(imei,12,#imei) )
	--local text = string.format("%s频道,道客[%s]，上线了。",channel_name, ok_nickname)
	--channelSet [channel] = channel

	local redis_eval_command = "local channel_idx = redis.call('get','%s:channelID')  if channel_idx and #tostring(channel_idx) >= 9 then local channel_name = redis.call('hget', tostring(channel_idx) .. ':userChannelInfo','channelName')  return channel_name end  return nil " 
	
	for i,v in pairs(channelSet) do

		local ok , channel_name = redis_api.cmd('private','eval',string.format(redis_eval_command, i ) , 0 )
		if ok and channel_name then
			local text = string.format("%s频道,道客%s，上线了。",channel_name, ok_nickname)
			local  ok_status,ok_url,ok_fileid = alone_utils.txt_2_voice(appInfo.appKey,appInfo.secret,text)
			if not ok_status or ok_url == nil then 
				APP_ONLY.log('D',string.format("accountID:%s text;%s  txt to voice failed!",accountID,text))
				return 
			end
			channelSet[i] = {}
			channelSet[i]['sourceID'] = ok_fileid
			channelSet[i]['content'] = text
			channelSet[i]['multimediaURL'] = ok_url
		end
	end

	----5分钟， level等级：21 微博格式：道客[道客昵称]，上线了。

	local tab = {
		appKey            = appInfo.appKey,
		level             = 80,
		interval          = 15,  --2014-09-11 有效时间修改为15s
		senderAccountID   = accountID,
		senderType        = 2,   ---------添加发送类型区分微博来源 1:WEME    2:system    3:other
	}
	for i,v in pairs(account_tab) do
		if v and weibo.check_appcenter_subscribed_msg(v['accountID'], weibo.DRI_APP_ONLINE_REPORT) then
			tab['receiverAccountID'] = v['accountID']
			tab['multimediaURL'] = tostring(channelSet[v['channel']]['multimediaURL'])
			tab['sourceID'] = tostring(channelSet[v['channel']]['sourceID'])
			tab['content'] = tostring(channelSet[v['channel']]['content'])
			tab['sign'] = utils.gen_sign(tab, appInfo.secret)
			local ok,bizid = alone_utils.really_send_multi_personal_weibo(tab)
			if ok then

				APP_ONLY.log('D',string.format("accountid:%s  to:%s  bizid: %s ", accountID, v['accountID'] , bizid ))

				---- 2015-06-04 请检查以下代码是否需要 -----------begin
				local time = os.time()
				local travelID  = nil 
				local appKey = appInfo.appKey
				local ok, imei =  redis_api.cmd('private', cur_accountid ,'get', string.format("%s:IMEI", accountID))
				if ok and imei then
					ok, travelID = redis_api.cmd('private', cur_accountid ,'get', string.format("%s:travelID", imei))
				end 
				local ok_date, cur_date = redis_api.cmd('private', cur_accountid ,'get', string.format("%s:speedDistribution", accountID))
				if not ok_date or not cur_date then
					cur_date = os.date("%Y%m")
					redis_api.cmd('private', cur_accountid ,'set', string.format("%s:speedDistribution", accountID), cur_date)
				end 
				---- 2015-06-04 请检查以下代码是否需要 -----------end 

			end
		end
	end
end


function work()
	APP_ONLY.log("I", "=========a_app_report_user_online =================")
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	work_online(accountID)
end

