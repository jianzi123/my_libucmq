
local only = require 'only'
local gosay = require 'gosay'
local utils = require 'utils'
local msg = require 'msg'
local weibo_fun = require 'weibo_fun'
local APP_POOL  = require('pool')
local mysql_api = require 'mysql_pool_api'
local redis_pool_api = require 'redis_pool_api'
module('weidb_update', package.seeall)

local app_newstatus_db = "app_ns___newStatus"
local app_weiboread_db = "app_weibo___weibo_read"

local gps_point_unit           = 10000000

local G = {
	
	sql_upate_read_group_weibo    = "UPDATE %s_%s SET readStatus=1, readTime = unix_timestamp() , readLongitude='%s', readLatitude='%s' WHERE readStatus = 0 and receiverAccountID ='%s' AND bizid='%s' ",
	sql_upate_read_personal_weibo = " UPDATE %s_%s SET readStatus=1, readTime= unix_timestamp() , readLongitude='%s', readLatitude='%s' WHERE readStatus = 0 and  bizid='%s' ",
}

local function save_read_bizid(weibo)
    
	local cur_month = os.date('%Y%m')
	local cur_time = os.time()
	
	local sql_tab = {
		a1 = 'readGroupTTSWeibo',
		a2 = 'readGroupWeibo',
		a3 = 'readPersonalTTSWeibo',
		a4 = 'readPersonalWeibo',
		a5 = 'readPersonalTTSWeibo',
		a6 = 'readPersonalWeibo',
	}
	
	---- 设置最新经度

	local longitude = tonumber(weibo['curLongitude']) or 0
	local latitude = tonumber(weibo['curLatitude']) or 0
	
	
	if not weibo['curBizid'] then
		only.log('E', "get read_bizid failed! %s" , weibo['curBizid']) 
		return
	end
	local bizid_tab = {}
	--table.insert(bizid_tab,weibo['curBizid'])
	bizid_tab = weibo['curBizid']

	---- 更新已经下发的微博的状态
	for _, v in pairs(bizid_tab) do

		local sql_str = nil
		local begin_bizid = string.sub(v, 1, 2)
		only.log('D',v)
		if begin_bizid == 'a1' or begin_bizid == 'a2' then
			---- a1 集团 TTS 
			---- a2 集团 语音
			sql_str = string.format(G.sql_upate_read_group_weibo,
						sql_tab[begin_bizid],
						 cur_month,
						 longitude * gps_point_unit ,
						 latitude * gps_point_unit ,
						 weibo['curAccountID'],
						 v)
		elseif begin_bizid == 'a3' or begin_bizid == 'a4' then
			---- a3 个人 TTS 
			---- a4 个人 语音
			sql_str = string.format(G.sql_upate_read_personal_weibo,
						 sql_tab[begin_bizid],
						 cur_month,
						 longitude * gps_point_unit ,
						 latitude * gps_point_unit ,
						 v)
		elseif begin_bizid == 'a5' or begin_bizid == 'a6' then
			---- a5 区域 TTS 
			---- a6 区域 语音
			sql_str = string.format(G.sql_upate_read_personal_weibo,
						 sql_tab[begin_bizid],
						 cur_month,
						 longitude * gps_point_unit ,
						 latitude * gps_point_unit ,
						 v)
		end
		if sql_str then
			local ok_status,ok_ret = mysql_api.cmd(app_weiboread_db,'UPDATE',sql_str)
			if not ok_status then
				only.log('E','save read weibo failed! %s ', sql_str )
			end
		end
	end
	return true
end

function handle()

	local tab = APP_POOL["OUR_BODY_TABLE"]

	if not tab then
		only.log('D',"MSG_ERROR_REQ_ARG_NO_BODY")
		return false
	end

	local	ok = save_read_bizid(tab)

	if not ok then
		only.log('S',"MSG_DO_MYSQL_FAILED")
		return false
	end
	return true
end








