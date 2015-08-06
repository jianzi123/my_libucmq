
local utils = require 'utils'
local link = require 'link'
local http_short_api = require 'http_short_api'

local dfs_srv_save_sound = link["OWN_DIED"]["http"]["dfsapi_save_sound"] 

-- update by chenjf
local only = require 'only'
local mysql_pool_api = require 'mysql_pool_api'
local redis_pool_api = require 'redis_pool_api'
-- end


local mapGrid = 'mapGridOnePercent'

module('weibo_fun', package.seeall)

function check_geometry_attr(lon, lat, alt, dist, dir, sped, owner, tab)

    if lon then
        lon = tonumber(lon)
        if not lon or lon<-180 or lon>180 then
            return false, owner .. "Longitude"
        end
    end

    if lat then
        lat = tonumber(lat)
        if not lat or lat<-90 or lat>90 then
            return false, owner .. "Latitude"
        end
    end

    if alt then
        alt = tonumber(alt)
        if not alt or alt<-9999 or alt>9999 then
            return false, owner .. "Altitude"
        end
    end

    if dist then
        dist = tonumber(dist)
        if not dist or dist < 0 then
            return false, owner .. "Distance"
        end
    end

	if owner == 'receiver' and lon and lat and not dist then
		return false, owner .. "Distance"
	end

    local dir_tab, speed_tab
    if dir then
        local ok, direction = utils.json_decode(dir)
        if not ok or type(direction) ~= 'table' then
            return false, owner .. "Direction"
        end
        if not direction[1] or (direction[1]<0 or direction[1]>360) and direction[1]~=-1 then
            return false, owner .. "Direction"
        end
        -- if not direction[2] or direction[2]<0 or direction[2]>180 then
        --     gosay.go_false(url_info, msg["MSG_ERROR_REQ_ARG"], owner .. "Direction")
        -- end
        dir_tab = direction
    end

    if sped then
        local ok, speed = utils.json_decode(sped)
        if not ok or type(speed)~='table' then
            return false, owner .. "Speed"
        end
        if #speed==0 or #speed>2 then
            return false, owner .. "Speed"
        end
        if tonumber(speed[1])==nil or tonumber(speed[1])<0 then
            return false, owner .. "Speed"
        end
        if speed[2] and (tonumber(speed[2])==nil or tonumber(speed[2])<0) then
            return false, owner .. "Speed"
        end
        speed[2] = speed[2] or 999
        speed_tab = speed
    end

    return true, dir_tab, speed_tab
end


function create_bizid(flag)
    local uuid = utils.create_uuid()
    local bizid = flag .. uuid
    return bizid
end

function get_dfs_url(tab, app_key)

    if not tab.data then
        return nil
    end

    local tab = {
        appKey = app_key,
        length = #tab.data,
    }

    local ok, secret = redis_pool_api.cmd('public', "", 'hget', wb['appKey'] .. ':appKeyInfo', secret)
    if not ok or not secret then
        return nil
    end

    wb['sign'] = utils.gen_sign(tab, secret)

    local http_req = utils.post_data(dfs_srv_save_sound['host'], dfs_srv_save_sound['port'], 'dfsapi/v2/saveSound', tab.file_name, tab.data)

    local ret = http_short_api.http(dfs_srv_save_sound, http_req, true)
    if not ret then
        return nil
    end

    local body = string.match(ret, '.-\r\n\r\n(.+)')

    only.log('D', body)
    local ok, data = utils.json_decode(body)

    if not ok then
        only.log('E', info)
        return nil
    end

    if tonumber(data["ERRORCODE"]) ~= 0 then
        only.log('E', body)
        return nil
    end

    return data['RESULT']['fileID'], data['RESULT']['url']
end

function touch_media_redis(args_tab)

    -- %s is special in redis, we should escape it

    local media_url = utils.escape_redis_text(args_tab['multimediaURL'])

    local js_tab = {
        multimediaFileURL = media_url,
        time = {args_tab['start_time'], args_tab['endTime']},
        tokenCode = args_tab['tokenCode'],

        content = args_tab['content'],
        bizid = args_tab['bizid'],
        level = args_tab['level'],
        type = args_tab['messageType'] or 1,
        longitude = args_tab['receiverLongitude'],
        latitude = args_tab['receiverLatitude'],
        distance = args_tab['receiverDistance'],
        direction = args_tab['direction_tab'],
        speed = args_tab['speed_tab'],
    }


    local key = string.format('%s:%s', args_tab['receiverAccountID'], 'weiboPriority')

    only.log('D', string.format('zadd %s %s %s', key, args_tab['level'] .. js_tab['time'][1],  js_tab['bizid']))
    local ok, ret = redis_pool_api.cmd('weibo', "", 'zadd', key, args_tab['level'].. js_tab['time'][1],  js_tab['bizid'])
    if not ok then
        only.log('E', string.format('zadd %s %s %s', key, args_tab['level'] .. js_tab['time'][1],  js_tab['bizid']))
        return false
    end

    key = string.format('%s:%s', js_tab['bizid'], 'weibo')
    local ok, val = utils.json_encode(js_tab)

    only.log('D', string.format('set %s %s', key, val))
    ok, ret = redis_pool_api.cmd('weibo', "", 'set', key, val)
    if not ok then
        only.log('E', string.format('set %s %s', key, val))
        return false
    end


    local sender_info = {
        senderAccountID = args_tab['senderAccountID'],
        callbackURL = args_tab['callbackURL'],
        sourceID = args_tab['sourceID'],
        fileID = args_tab['fileID'],
        appKey = args_tab['appKey'],
        commentID = args_tab['commentID'],
    }

    if next(sender_info) then
        local ok, json_string = utils.json_encode(sender_info)
        if not ok then
            return false
        end
        only.log('D', string.format('set %s %s', args_tab['bizid'] .. ':senderInfo', json_string))
        ok, ret = redis_pool_api.cmd('weibo', "", 'set', args_tab['bizid'] .. ':senderInfo', json_string)
        if not ok then
            only.log('E', string.format('set %s %s', args_tab['bizid'] .. ':senderInfo', json_string))
            return false
        end

        only.log('D', string.format('expire %s %s', args_tab['bizid'] .. ':senderInfo', args_tab['interval']+300))
        ok, ret = redis_pool_api.cmd('weibo', "", 'expire', args_tab['bizid'] .. ':senderInfo', args_tab['interval']+300)
        if not ok then
            only.log('E', string.format('expire %s %s', args_tab['bizid'] .. ':senderInfo', args_tab['interval']+300))
            return false
        end
    end

    return true
end


function touch_media_db(args)

    local sql_fmt = "INSERT personalMultimediaWeibo_%s SET appKey='%s',sourceID='%s',sourceType=%s,geometryType=%d,multimediaURL='%s'," ..
    "senderAccountID='%s',senderLongitude=%s,senderLatitude=%s,senderDirection=%s,senderSpeed=%s,senderAltitude=%s,commentID='%s'," ..
    "receiverAccountID='%s',receiverLongitude=%s,receiverLatitude=%s,receiverDirection='%s',receiverSpeed='%s',receiverDistance=%s,content='%s'," ..
    "endTime=%d,level=%d,tokenCode='%s',callbackURL='%s',createTime=%d,bizid='%s',cityCode=%s,cityName='%s',senderType=%d, messageType=%d,autoReply=%s,invalidDis=%s,tipType=%s,POIID='%s',POIType='%s'"

    local cur_month = os.date('%Y%m')

    local media_url = utils.escape_mysql_text(args['multimediaURL'])
    local sender_lon = args['senderLongitude'] and args['senderLongitude'] * 10000000 or 0 
    local sender_lat = args['senderLatitude'] and args['senderLatitude'] * 10000000 or 0 
    local receiver_lon = args['receiverLongitude'] and args['receiverLongitude'] * 10000000 or 0 
    local receiver_lat = args['receiverLatitude'] and args['receiverLatitude'] * 10000000 or 0 

    local city_code, city_name, json_tab
    if sender_lon~=0 and sender_lat~=0 then
        local grid_no = string.format('%d&%d', math.floor(args['senderLongitude']*100), math.floor(args['senderLatitude'] * 100))

        only.log('D', grid_no)
        local ok, ret = redis_pool_api.cmd(mapGrid, "", 'get', grid_no)
        if ok then
            ok, json_tab = utils.json_decode(ret)
        end
        if ok then
            city_code, city_name = json_tab['cityCode'], json_tab['cityName']
        end
    end
    

    local sql = string.format(sql_fmt, cur_month, args['appKey'], args['sourceID'] or '', args['sourceType'], args['geometryType'] or 0,
     media_url, args['senderAccountID'] or '', sender_lon, sender_lat,
    args['senderDirection'] or 0, args['senderSpeed'] or 0, args['senderAltitude'] or -9999, args['commentID'] or '',
    args['receiverAccountID'] or '', receiver_lon, receiver_lat, args['receiverDirection'] or '', args['receiverSpeed'] or 0,
    args['receiverDistance'] or 0, args['content'] or '', args['endTime'], args['level'], 
    args['tokenCode'] or '', args['callbackURL'] or '', os.time(), args['bizid'], city_code or 0, city_name or '', args['senderType'] or 3, args['messageType'] or 1,args['autoReply'] or 0,args['invalidDis'] or 0,args['tipType'] or 0,args['POIID'] or '',args['POIType'] or '')

    only.log('D', sql)
    local ok, ret = mysql_pool_api.cmd('app_weibo___weibo', 'INSERT', sql)
    if not ok then
        only.log('E', sql)
        return false
    end

    return true
end


function touch_comment_db(args)

    local sql_fmt = "INSERT multimediaCommentInfo_%s SET senderAccountID='%s',receiverAccountID='%s',fileURL='%s'," ..
                    "sourceID='%s',appKey='%s',callbackURL='%s',createTime=%d,longitude=%s,latitude=%s,direction= %s," ..
                    "speed= %s,altitude=%s,bizid='%s',commentID='%s'"

    local cur_month = os.date('%Y%m')

    local media_url = utils.escape_mysql_text(args['multimediaURL'])
    local sender_lon = args['senderLongitude'] and args['senderLongitude'] * 10000000 or 0 
    local sender_lat = args['senderLatitude'] and args['senderLatitude'] * 10000000 or 0 

    local sql = string.format(sql_fmt, cur_month, args['senderAccountID'] or '', args['receiverAccountID'] or '', media_url,
    args['sourceID'] or '', args['appKey'], args['callbackURL'] or '', os.time(), sender_lon, sender_lat, args['senderDirection'] or 0,
    args['senderSpeed'] or 0, args['senderAltitude'] or -9999, args['bizid'],args['commentID'] or '')

    only.log('D', sql)
    local ok, ret = mysql_pool_api.cmd('app_weibo___weibo', 'INSERT', sql)
    if not ok then
        only.log('E', sql)
        return false
    end
    return true
end

function get_region_name(region_code)
    if not region_code then
        return nil
    end
    local sql = string.format("SELECT name FROM chinaDistrictInfo WHERE code = %s",region_code)
    only.log('D',sql)
    local ok,ret = mysql_pool_api.cmd("app_roadmap___roadmap", "select", sql)

    if not ok or #ret < 1 then
        only.log('E', "select mysql daokemap  chinaDistrictInfo error!")
        return nil
    else
        return ret[1]['name']
    end

end

------统计变量修改时间2014-10-16---------
----统计所有appKey发送微博的总记录数
function incrby_appkey(appKey, cur_month, cur_day)
    --HINCRBY myhashtable field 1
    --201410:weiboAppKeyTotalInfo
    local ok = redis_pool_api.cmd('weiboStore', "", 'HINCRBY', string.format('%s:weiboAppKeyTotalInfo', cur_day) , string.format("%s:totalNum", appKey ) , 1 )
    if not ok then
        only.log('E',string.format("incrby_appkey  %s  cur_month: %s  cur_day:%s failed! ", appKey ,cur_month, cur_day))
    end
end

----统计群组集团微博发送次数
function incrby_groupid( groupid, cur_month, cur_day )
    --201410:weiboGroupIDTotalInfo
    local ok = redis_pool_api.cmd('weiboStore', "", 'HINCRBY', string.format('%s:weiboGroupIDTotalInfo', cur_day) , string.format("%s:totalNum", groupid ) , 1 )
    if not ok then
        only.log('E',string.format("incrby_groupid  %s  cur_month: %s  cur_day:%s failed! ", groupid ,cur_month, cur_day))
    end
end

----统计发送个人区域微博发送次数
function incrby_citycode( city_code, cur_month, cur_day )
    --201410:weiboGroupIDTotalInfo
    local ok = redis_pool_api.cmd('weiboStore', "", 'HINCRBY', string.format('%s:weiboCityCodeTotalInfo', cur_day) , string.format("%s:totalNum", city_code ) , 1 )
    if not ok then
        only.log('E',string.format("incrby_citycode  %s  cur_month: %s  cur_day:%s failed! ", groupid ,cur_month, cur_day))
    end
end

---- 统计发送个人微博总数
function incrby_personal( cur_month, cur_day , cur_count)
    --201410:weiboGroupIDTotalInfo
    local ok = redis_pool_api.cmd('weiboStore', "", 'HINCRBY', string.format('%s:weiboPersonalTotalInfo', cur_month) , string.format("%s:totalNum", cur_day ) , tonumber(cur_count) or 1 )
    if not ok then
        only.log('E',string.format("incrby_personal  %s  cur_month: %s  cur_day:%s failed! ", groupid ,cur_month, cur_day))
    end
end

---- 群组最后一次说话时间
function groupid_update_timestamp( groupid )
    local ok = redis_pool_api.cmd('weiboStore', "", 'hmset', "groupIDUpdateTimestamp", string.format("%s:timestamp" , groupid ) ,os.time())
    if not ok then
        only.log('E',string.format("incrby_personal  %s  cur_month: %s  cur_day:%s failed! ", groupid ,cur_month, cur_day))
    end
end

------统计变量修改时间2014-10-16---------
