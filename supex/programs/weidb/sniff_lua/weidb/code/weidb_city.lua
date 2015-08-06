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

module('weidb_city', package.seeall)

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
	
	if tab['receiveCrowd'] and type(tab['receiveCrowd']) == 'table' then 
	 	 ok,tab['receiveCrowd'] = utils.json_encode(tab['receiveCrowd'])
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

	return true
end

local function touch_db(args)
    
	local sql_fmt = "INSERT regionWeibo_%s SET appKey=%s,sourceID='%s',sourceType=%s,multimediaURL='%s'," ..
    "senderAccountID='%s',senderLongitude=%s,senderLatitude=%s,senderDirection=%s ,senderSpeed=%s ,senderAltitude=%s,regionCode=%s,regionName='%s'," ..
    "receiveCrowd='%s',receiveSelf=%s,receiverLongitude=%s,receiverLatitude=%s,receiverDirection='%s',receiverSpeed='%s',receiverDistance=%s," ..
    "content='%s',endTime=%s,level=%s,tokenCode='%s',callbackURL='%s',appendFileURL='%s',createTime=%s,bizid='%s',messageType=%s,autoReply=%s,invalidDis=%s,tipType=%s"

    local cur_month = os.date('%Y%m')

    local media_url = utils.escape_mysql_text(args['multimediaURL'])
    local sender_lon = args['senderLongitude'] and args['senderLongitude'] * 10000000 or 0 
    local sender_lat = args['senderLatitude'] and args['senderLatitude'] * 10000000 or 0 
    local receiver_lon = args['receiverLongitude'] and args['receiverLongitude'] * 10000000 or 0 
    local receiver_lat = args['receiverLatitude'] and args['receiverLatitude'] * 10000000 or 0 

    -->> get region name
    local region_name = weibo_fun.get_region_name(tonumber(args['regionCode']))
    if not region_name then
	only.log('S',"MSG_DO_MYSQL_FAILED")
	return false
    end

	local sql = string.format(sql_fmt, cur_month, args['appKey'], args['sourceID'] or '', args['sourceType'], media_url,
    args['senderAccountID'] or '', sender_lon, sender_lat, args['senderDirection'] or 0, args['senderSpeed'] or 0, args['senderAltitude'] or -9999, args['regionCode']or 0, region_name or '',
    args['receiveCrowd'] or '', args['receiveSelf'], receiver_lon, receiver_lat, args['receiverDirection'] or '', args['receiverSpeed'] or 0, args['receiverDistance'] or 0,
    args['content'] or '',args['endTime'], args['level'], args['tokenCode'] or '', args['callbackURL'] or '',args['appendFileURL'] or '', os.time(), args['bizid'], args['messageType'] or 0,args['autoReply'] or 0,args['invalidDis'] or 0,args['tipType'] or 0)

    only.log('D', sql)
    local ok, ret = mysql_pool_api.cmd('app_weibo___weibo', 'INSERT', sql)
    if not ok then
        only.log('S',"MSG_DO_MYSQL_FAILED")
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

	local ok = touch_db(tab)
	if not ok then
		only.log('S',"MSG_DO_MYSQL_FAILED")
		return false
	end

	local cur_month = os.date('%Y%m')
	local cur_day = os.date("%Y%m%d")
    weibo_fun.incrby_appkey( tab['appKey'], cur_month, cur_day)
    weibo_fun.incrby_citycode( tab['regionCode'], cur_month, cur_day)

end

