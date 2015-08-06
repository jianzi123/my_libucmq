--
-- author: houyaqian
-- date: 2015 01 12 Mon 17:19:00 CST

local msg = require('msg')
local only = require('only')
local gosay = require('gosay')
local utils = require('utils')
local weibo_fun = require('weibo_fun')
local APP_POOL  = require('pool')
local mysql_pool_api = require('mysql_pool_api')
local redis_pool_api = require('redis_pool_api')

module('weidb_personal', package.seeall)

local function check_parameters(tab)
	
	tab['senderDirection'] = tonumber(tab['senderDirection'])
	if tab['senderDirection'] and (tab['senderDirection'] <0 or tab['senderDirection'] >360) and tab['senderDirection'] ~= -1 then 
	only.log('D', "senderDirection is error")
	return false
	end

	tab['senderSpeed'] = tonumber(tab['senderSpeed'])
	if tab['senderSpeed'] and tab['senderSpeed']<0 then 
	only.log('D', "senderSpeed is error")
	return false
	end

	if tab['receiverDirection'] and type(tab['receiverDirection']) == 'table' then 
	 	 ok,tab['receiverDirection'] = utils.json_encode(tab['receiverDirection'])
    end

	if tab['receiverSpeed'] and type(tab['receiverSpeed']) == 'table' then 
	 	 ok,tab['receiverSpeed'] = utils.json_encode(tab['receiverSpeed'])
    end

    local ok ,res =  weibo_fun.check_geometry_attr(tab['senderLongitude'], tab['senderLatitude'], tab['senderAltitude'], tab['senderDistance'],nil,nil, 'sender')
	
	if not ok then
			only.log('D',string.format("%s is error",res))
			return false
	end
	 
	 tab['senderType'] = tonumber(tab['senderType']) or 3
     if tab['senderType']<1 or tab['senderType']>3 then
			only.log('D', "senderType is error")
			return false
     end


	if tab['POIID'] and (string.sub(tab['POIID'],1,1) ~='P' or #tab['POIID'] ~=9) then
			only.log('D', "POIID is error")
			return false
	end

	if tab['POIType']  and  #tab['POIType'] ~= 6 then
			only.log('D', "POIType is error")
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

	local ok = check_parameters(tab)
	if not ok then
		return false
	end

	local ok = weibo_fun.touch_media_db(tab)
	if not ok then
		only.log('S',"MSG_DO_MYSQL_FAILED")
		return false
	end

	if tab['commentID'] then
		local ok = weibo_fun.touch_comment_db(tab)
		if not ok then
			only.log('S',"MSG_DO_MYSQL_FAILED")
			return false
		end
	end

	local cur_month = os.date('%Y%m')
	local cur_day = os.date("%Y%m%d")
	weibo_fun.incrby_appkey(tab['appKey'], cur_month, cur_day)
	weibo_fun.incrby_personal( cur_month, cur_day)

end

