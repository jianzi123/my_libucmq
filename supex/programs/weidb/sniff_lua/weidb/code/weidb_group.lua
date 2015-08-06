-- author: houyaqian
-- date: 2015-01-17

local only = require 'only'
local gosay = require 'gosay'
local utils = require 'utils'
local msg = require 'msg'
local weibo_fun = require 'weibo_fun'
local APP_POOL  = require('pool')
local mysql_pool_api = require 'mysql_pool_api'
local redis_pool_api = require 'redis_pool_api'
module('weidb_group', package.seeall)

local function touch_db(args)

    local sql_fmt = "INSERT groupMultimediaWeibo_%s SET appKey=%s,sourceID='%s',sourceType=%s,multimediaURL='%s'," ..
    "senderAccountID='%s',senderLongitude=%s,senderLatitude=%s,senderDirection=%s ,senderSpeed=%s ,senderAltitude=%s,cityCode=%s,cityName='%s'," ..
    "groupID='%s',receiveCrowd='%s',receiveSelf=%s,receiverLongitude=%s,receiverLatitude=%s,receiverDirection='%s',receiverSpeed='%s',receiverDistance=%s," ..
    "content='%s',endTime=%d,level=%d,tokenCode='%s',callbackURL='%s',appendFileURL='%s',createTime=%d,bizid='%s',messageType=%d,autoReply=%d,invalidDis=%d,tipType=%d"

    local cur_month = os.date('%Y%m')

    local media_url = utils.escape_mysql_text(args['multimediaURL'])
    local sender_lon = args['senderLongitude'] and args['senderLongitude'] * 10000000 or 0 
    local sender_lat = args['senderLatitude'] and args['senderLatitude'] * 10000000 or 0 
    local receiver_lon = args['receiverLongitude'] and args['receiverLongitude'] * 10000000 or 0 
    local receiver_lat = args['receiverLatitude'] and args['receiverLatitude'] * 10000000 or 0 

    local city_code, city_name, json_args
    if sender_lon~=0 and sender_lat~=0 then
        local grid_no = string.format('%d&%d', math.floor(args['senderLongitude']*100), math.floor(args['senderLatitude'] * 100))

        only.log('D', grid_no)
        local ok, ret = redis_pool_api.cmd('mapGridOnePercent', "", 'get', grid_no)
        if ok then
            city_code = ret 
            ok, json_args = utils.json_decode(ret)
        end 
        if ok then
            city_code, city_name = json_args['cityCode'], json_args['cityName']
        end 
    end
	local ok,receiveCrowd
	if args['receiveCrowd'] and type(args['receiveCrowd']) == 'table' then 
	 	 ok,receiveCrowd = utils.json_encode(args['receiveCrowd'])
	else
		receiveCrowd = args['receiveCrowd']
    end


	local sql = string.format(sql_fmt, cur_month, args['appKey'], args['sourceID'] or '', args['sourceType'], media_url,
    args['senderAccountID'] or '', sender_lon, sender_lat, args['senderDirection'] or 0, args['senderSpeed'] or 0, args['senderAltitude'] or -9999, city_code or 0, city_name or '',
    args['groupID'] or '', receiveCrowd or '', args['receiveSelf'], receiver_lon, receiver_lat, args['receiverDirection'] or '', args['receiverSpeed'] or 0, args['receiverDistance'] or 0,
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

	local args = APP_POOL["OUR_BODY_TABLE"]
	if not args then
		only.log('D',"THE BODY IS EMPTY")
		return false
	end
	
	
	local ok

	if args['receiverDirection'] and type(args['receiverDirection']) == 'table' then 
	 	 ok,args['receiverDirection'] = utils.json_encode(args['receiverDirection'])
    end

	if args['receiverSpeed'] and type(args['receiverSpeed']) == 'table' then 
	 	 ok,args['receiverSpeed'] = utils.json_encode(args['receiverSpeed'])
    end

	args['senderDirection'] = tonumber(args['senderDirection'])
	if args['senderDirection'] and (args['senderDirection'] <0 or args['senderDirection'] >360) and args['senderDirection'] ~= -1 then 
		only.log('D', "senderDirection is error")
		return false
	end

	args['senderSpeed'] = tonumber(args['senderSpeed'])
	if args['senderSpeed'] and args['senderSpeed'] <0 then 
		only.log('D', "senderSpeed is error")
		return false
	end

	local ok ,res =  weibo_fun.check_geometry_attr(args['senderLongitude'], args['senderLatitude'], args['senderAltitude'], args['senderDistance'], nil , nil, 'sender')

	if not ok then
		only.log('D',string.format("%s is error",res))
		return false
	end
	
	ok = touch_db(args)
	
	if not ok then 
		only.log('D',"MySQL DO FAILED")
		return false
	end

	 ---- 统计appkey发送微博总数
     local cur_month = os.date('%Y%m')
     local cur_day = os.date("%Y%m%d")
     weibo_fun.incrby_appkey( args['appKey'], cur_month, cur_day)
     weibo_fun.incrby_groupid( args['groupID'], cur_month, cur_day)
 
     ----频道内最后一次说话时间
     weibo_fun.groupid_update_timestamp( args['groupID'] )

end

