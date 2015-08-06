
local only = require 'only'
local gosay = require 'gosay'
local utils = require 'utils'
local msg = require 'msg'
local weibo_fun = require 'weibo_fun'
local APP_POOL  = require('pool')
local mysql_api = require 'mysql_pool_api'
local redis_pool_api = require 'redis_pool_api'
module('weidb_issued', package.seeall)

local app_newstatus_db = "app_ns___newStatus"
local app_weiboread_db = "app_weibo___weibo_read"

local gps_point_unit           = 10000000

local function save_read_weibo_info(tab)
	local sql_tab = {
		a1 = 'readGroupTTSWeibo',
		a2 = 'readGroupWeibo',
		a3 = 'readPersonalTTSWeibo',
		a4 = 'readPersonalWeibo',
		a5 = 'readPersonalTTSWeibo',
		a6 = 'readPersonalWeibo',
	}

	local weibo = tab['weibo_info']
	local sender_info = weibo['sender_info']	

	local bizid = weibo['bizid']
	local tb_type = string.sub(bizid,1,2)
        local cur_month = os.date('%Y%m')
	local cur_time = os.time()
	local sql_table = string.format("%s_%s",sql_tab[tb_type], cur_month )
	

	local sender_accountID , callback_url, sender_fileid = '','',''
	local appKey = 0
	local commentID = ''
	
	sender_accountID = sender_info['senderAccountID'] or ''
	sender_fileid    = sender_info['sourceID'] or ''
	callback_url     = sender_info['callbackURL'] or ''
	appKey           = tonumber(sender_info['appKey']) or 0
	commentID        = sender_info['commentID'] or ''
	
	local speed_str = ""
	if weibo['speed'] and type(weibo['speed']) == "table" then
		speed_str = string.format("[%s]",table.concat(weibo['speed'],","))
	end

	local direction_str = ""
	if weibo['direction'] and type(weibo['direction']) == "table" then
		direction_str = string.format("[%s]",table.concat(weibo['direction'],","))
	end
	
	local sql_str =  string.format("insert into %s set senderAccountID = '%s', sourceID = '%s' , bizid  = '%s'  , releaseTime = %s " ..
			" , readStatus = %s , releaseLongitude = %s , releaseLatitude = %s  " ..
			" , level = %s , content = '%s' , callbackURL = '%s' " ..
			" , speed = '%s'  , longitude = %s , latitude = %s  , distance = %s , direction = '%s' , tokenCode = '%s' "  ..
			" , appKey = '%s', autoReply =%s , tipType = %s, invalidDis= %s " ,
			sql_table,sender_accountID,sender_fileid,weibo['bizid'],cur_time ,0,
			(tonumber(weibo['releaseLongitude']) or 0)  * gps_point_unit,
			(tonumber(weibo['releaseLatitude']) or 0)  * gps_point_unit,
			(tonumber(weibo['level']) or 99),
			weibo['content'] or '',
			callback_url,
			speed_str,
			(tonumber(weibo['longitude']) or 0)  * gps_point_unit ,
			(tonumber(weibo['latitude']) or 0)  * gps_point_unit ,
			(tonumber(weibo['distance']) or 0) ,
			direction_str,
			weibo['tokencode'] or '',
			appKey,
			(tonumber(weibo['autoReply']) or 0),
			(tonumber(weibo['tipType']) or 0),
			(tonumber(weibo['invalidDis']) or 0))

	if tb_type == 'a1' or tb_type == 'a2' then
		sql_str = sql_str .. string.format("  , receiverAccountID = '%s' , groupID = '%s', multimediaURL = '%s'   " ,
			tab['curAccountID'],
			weibo['groupID'],
			weibo['mediaFileURL'] or '' )

	elseif tb_type == 'a3' or tb_type == 'a4' then
	---- 个人微博
		sql_str = sql_str .. string.format(" , commentID = '%s' , receiverAccountID = '%s' ,POIID = '%s', POIType = '%s' " , commentID,  tab['curAccountID'] ,weibo['POIID'],weibo['POIType'])
		if tb_type == 'a4' then
		local multimediaFile_url = weibo['mediaFileURL'] or ''
		local multimediaFile_type = 'amr'
		if #multimediaFile_url  > 5 then
			local tmp_url = string.sub(multimediaFile_url,-5,-1)
			local find_dot = string.find(tmp_url,'%.')
			if find_dot then
				multimediaFile_type = string.sub(tmp_url,find_dot+1,#multimediaFile_url)
			end
		end
		sql_str = sql_str .. string.format(" , multimediaURL = '%s' , fileType = '%s'   " ,
				multimediaFile_url,
				multimediaFile_type )
	end
	elseif tb_type == 'a5' or tb_type == 'a6' then
	---- 区域微博
		sql_str = sql_str .. string.format(" , commentID = '%s' , receiverAccountID = '%s'   " , commentID, tab['curAccountID'])
		if tb_type == 'a6' then
			local multimediaFile_url = weibo['mediaFileURL'] or ''
			local multimediaFile_type = 'amr'
			if #multimediaFile_url  > 5 then
				local tmp_url = string.sub(multimediaFile_url,-5,-1)
				local find_dot = string.find(tmp_url,'%.')
				if find_dot then
					multimediaFile_type = string.sub(tmp_url,find_dot+1,#multimediaFile_url)
				end
			end
			sql_str = sql_str .. string.format(" , multimediaURL = '%s' , fileType = '%s'   " ,
				multimediaFile_url,
				multimediaFile_type )
		end
	end

	local ok_status,ok_ret = mysql_api.cmd(app_weiboread_db,'INSERT',sql_str)
	if not ok_status then
		only.log('E', 'save read weibo failed!  %s  ', sql_str )
		return false
	end
		return true
end


function handle()

	local tab = APP_POOL["OUR_BODY_TABLE"]
	if not tab then
		only.log('D',"MSG_ERROR_REQ_ARG_NO_BODY")
		return false
	end

	local ok = save_read_weibo_info(tab)

	if not ok then
		only.log('S',"MSG_DO_MYSQL_FAILED")
		return false
	end
	return true
end








