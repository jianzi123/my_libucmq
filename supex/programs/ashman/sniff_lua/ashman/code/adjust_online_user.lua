local only            = require('only')
local gosay           = require('gosay')
local msg             = require('msg')
local safe            = require('safe')
local link            = require('link')
local utils           = require('utils')
local json            = require('cjson')
local scan            = require('scan')
local socket          = require('socket')
local pool            = require('pool')
local http_api        = require('http_short_api')
local redis_api       = require('redis_pool_api')

local dataCore = link["OWN_DIED"]["http"]['dataCore']
local online_cfg = {
	appKey = '3299949562',
        secret = '2A055C8CBE939BF3F3AB5BA525052BDA0DBE89B4',
}                                                                                                                                                          
local default_channel = "10086"
local VOICE_COMMAND_CHANNEL = 2 
module("adjust_online_user",package.seeall)


local function post_offline_status_to_datacore(accountid)
         if not accountid then return false end
         local ok_status,ok_tokencode = nil,nil
 
         local imei_val = nil
         if #accountid == 15 then ---is imei
                 ok_status,ok_tokencode = redis_api.cmd('private', accountid, 'get',accountid .. ':tokenCode')
                 if not ok_status or ok_tokencode  == nil then
                         only.log('D',string.format("accountID:%s get tokenCode failed!",accountid))
                         return false
                 end
                 imei_val = accountid
         elseif #accountid == 10 then
                 local ok,imei = redis_api.cmd('private', accountid, 'get',accountid .. ':IMEI')
                 if not ok or imei == nil then
                         only.log('D',string.format("accountID:%s get imei failed,so tokenCode failed!",accountid))
                         return false
                 end
                 imei_val = imei
                 ok_status,ok_tokencode = redis_api.cmd('private', accountid, 'get',imei .. ':tokenCode')
                 if not ok_status or ok_tokencode  == nil then                                                                    
                         only.log('D',string.format("accountID:%s get imei succ,but tokenCode failed!",accountid))
                         return false
                 end
         else
                 only.log('D',string.format("accountID:%s is error!",accountid))
                 return false
         end
 
         local args = {
                 accountID = accountid,
                 imei      = imei_val,
                 tokenCode = ok_tokencode ,
                 appKey    = online_cfg.appKey,
                 normal    = 0,  ----1正常关机 0非正常关机
         }
 
         local ok_status, travelID =  redis_api.cmd('private', accountid, 'get',string.format("%s:travelID",imei_val))
         if ok_status and travelID then
                 args['travelID'] = travelID
         end
 
         local url = 'DataCore/realTime/powerOff.do'
         args['sign'] = utils.gen_sign(args, secret)
 
         local post_urlencode = 'POST /%s HTTP/1.0\r\n' ..
                         'Host:%s:%s\r\n' ..
                         'Content-Length:%d\r\n' ..
                         'Content-Type:application/x-www-form-urlencoded\r\n\r\n%s'
 
         local body   = utils.table_to_kv(args)
         local data   = string.format(post_urlencode, url, dataCore.host, dataCore.port , #body , body)
 
         only.log('W',data)
 
         local ok = http_api.http_ex(dataCore, data, false,'DATACORE_SERVER',60)
         if not ok then
                 only.log('E',string.format("offline_status_to_datacore failed host:%s  port:%s url:%s ",dataCore.host, dataCore.port,url))
                 return false
         end
         return true
 
end

local function remove_user_from_tempchannel_set( accountid )
        local ok_status,ok_channel = redis_api.cmd('private', accountid, 'get',string.format('%s:tempChannel:groupVoice',accountid))
        if not ok_status or not ok_channel then
                return false
        end
        only.log('D',string.format("remove tempChannel accountid:%s,channelid;%s",accountid,ok_channel))
        redis_api.cmd('statistic', accountid, 'srem', string.format('%s:channelOnlineUser',ok_channel),accountid)
end                  

local function remove_user_from_groupvoice_set(accountid)
        local ok_status,ok_channel = redis_api.cmd('private', accountid, 'get',string.format('%s:currentChannel:groupVoice',accountid))
        if not ok_status then
                return false                                                                                                     
        end

        ok_channel = ok_channel or default_channel
        only.log('D',string.format("remove groupvoice accountid:%s,channelid;%s",accountid,ok_channel))
        redis_api.cmd('statistic', accountid, 'srem', string.format('%s:channelOnlineUser',ok_channel),accountid)
end


local function remove_user_from_voicecommand_set(accountid)
        local ok_status,ok_channel = redis_api.cmd('private', accountid, 'get',string.format('%s:currentChannel:voiceCommand',accountid))
        if not ok_status then
                return false
        end

        ok_channel = ok_channel or default_channel

        only.log('D',string.format("remove voicecommand accountid:%s,channelid;%s",accountid,ok_channel))
        redis_api.cmd('statistic', accountid, 'srem', string.format('%s:channelOnlineUser',ok_channel),accountid)
end

local function remove_user_from_citylist(accountid)
        local ok_status,ok_citycode = redis_api.cmd('private', accountid, 'get',accountid .. ':cityCode')                                    
        if not ok_status or ok_citycode == nil then
                return false
        end
        redis_api.cmd('statistic', accountid, 'srem', ok_citycode .. ':cityOnlineUser',accountid)
end

local function remove_user_from_mapgpsdata(accountid)                                                                            
        if not accountid or #tostring(accountid) ~= 10 then
                return false
        end
        local ok_status, ok_ret = redis_api.cmd('mapGPSData', accountid, 'hget', string.format("%s:info" , accountid ) , "gridID" )
        if ok_status and ok_ret then
                local ok, ret = redis_api.cmd('mapGPSData', accountid, 'srem', string.format("%s:accountIds", ok_ret ) , accountid )
                if ok then
                        redis_api.cmd('mapGPSData', accountid, 'del', string.format("%s:info" , accountid ) )
                        return true
                end
        end
        return false
end

function get_currentbl( accountid )
        local ok,gps_str = redis_api.cmd('private', accountid, 'get',accountid .. ":currentBL")
        if not ok or not gps_str then
                return false,nil,nil
        end
        local gps_info = utils.str_split(gps_str,",")
        if not gps_info or #gps_info ~= 2 then return false,nil,nil end

        local longitude = gps_info[1]
        local latitude = gps_info[2]

        if tonumber(longitude) == 0 or tonumber(latitude) == 0 then
                return false,nil,nil
        end
        return true, longitude,latitude
end

function get_city_info_by_bl(longitude, latitude)
        if not longitude or not latitude then return false,nil,nil end
        local grid = string.format("%d&%d",tonumber(longitude) * 100, tonumber(latitude) * 100 )

        local ok, result = redis_api.cmd('mapGridOnePercentV2', "", 'hget', grid ,'cityCode')
        if ok and result then
               
                        return true,result
                end
        
        only.log('W',string.format("err mapGridOnePercentV2 hget cityCode by grid %s failed!--->---",grid))
        return false,nil
end

function get_city_code_by_accountid(accountid)
        local ok , longitude, latitude = get_currentbl(accountid)                                                                                       
        if not ok then
                return false,nil,nil
        end
        return get_city_info_by_bl(longitude,latitude)
end

local function update_user_citycode(accountid)

        ---- 最新cityCode 
        local ok_status, cur_citycode = redis_api.cmd('private', accountid, 'get', accountid .. ':cityCode')
        if not ok_status then
                return false
        end

        ---- 最新经纬度对应的cityCode
        --local ok , new_citycode = appfun.get_city_code_by_accountid(accountid)
	local ok,new_citycode = get_city_code_by_accountid(accountid)
        if not ok then
                ---- redis异常
                if cur_citycode then
                        redis_api.cmd('statistic', accountid, 'sadd', cur_citycode .. ':cityOnlineUser', accountid)
                end
                return false
        end

        if not cur_citycode and new_citycode then
                ---- 最新经纬度对应的cityCode
                redis_api.cmd('statistic', accountid, 'sadd', new_citycode .. ':cityOnlineUser', accountid)
                redis_api.cmd('private', accountid, 'set', new_citycode .. ":cityCode", new_citycode)
        elseif cur_citycode and not new_citycode then
                redis_api.cmd('statistic', accountid, 'sadd', cur_citycode .. ':cityOnlineUser', accountid)
        end

        if cur_citycode and new_citycode then
                if cur_citycode == new_citycode then
                        ---- 当前城市编码与最新经纬度对应的cityCode一致,可能掉线了.需要sadd
                        redis_api.cmd('statistic', accountid, 'sadd', new_citycode .. ':cityOnlineUser', accountid)
                else
                        redis_api.cmd('private', accountid, 'set', accountid .. ":cityCode", new_citycode)
                        local ok, ret = redis_api.cmd('statistic', accountid, 'sismember',cur_citycode .. ':cityOnlineUser', accountid)
                        if ok and ret then
                                ---- 用户在之前的cityCode列表内,移动key对应的成员
                                redis_api.cmd('statistic', accountid, 'SMOVE',cur_citycode .. ':cityOnlineUser', new_citycode .. ':cityOnlineUser',accountid)
                        else                                                                                                     
                                ---- 用户不在之前的列表,直接sadd
                                redis_api.cmd('statistic', accountid, 'sadd', new_citycode .. ':cityOnlineUser', accountid)
                        end
                end
        end

end

local function update_user_channel_list( accountid )                                                                             

        local ok_status, ok_tmp_channel = redis_api.cmd('private', accountid, 'get',accountid .. ':tempChannel:groupVoice')
        if ok_status and ok_tmp_channel and #tostring(ok_tmp_channel) > 2 then
                ---- ++按键优先临时频道
                local tmp_channle_key = string.format("%s:channelOnlineUser",ok_tmp_channel)
                redis_api.cmd('statistic', accountid, 'sadd', tmp_channle_key, accountid)
        else
                local ok_status,ok_channel = redis_api.cmd('private', accountid, 'get',accountid .. ':currentChannel:groupVoice')
                ok_channel = ok_channel or default_channel
                redis_api.cmd('statistic', accountid, 'sadd', ok_channel .. ':channelOnlineUser',accountid)
        end

        local ok_status,ok_voice_type = redis_api.cmd('private', accountid, 'get',accountid .. ':voiceCommandCustomType')
        if ok_status and ok_voice_type then
                if tonumber(ok_voice_type) == VOICE_COMMAND_CHANNEL then
                        local ok_status,ok_channel = redis_api.cmd('private', accountid, 'get',accountid .. ':currentChannel:voiceCommand')
                        ok_channel = ok_channel or default_channel
                        redis_api.cmd('statistic', accountid, 'sadd', ok_channel .. ':channelOnlineUser',accountid)
                end
        end
        
end

local function update_user_from_follow_channellist( accountid )
        if not accountid  or #tostring(accountid) ~= 10 then return end

        local ok, ret = redis_api.cmd('private', accountid, 'smembers', string.format("%s:userFollowMicroChannel",accountid))
        if not ok or not ret or #ret < 1 then
                return true
        end

        local count  = #ret
        if count > 5 then
                local random = os.time() % count
                local check_channel_id = ret[random]
                if check_channel_id then
                        local ok, is_online = redis_api.cmd('statistic', accountid, 'sismember',string.format("%s:channelOnlineUser",check_channel_id),accountid)
                        if ok and is_online == true then
                                only.log('W', string.format("accountid:%s is online %s:channelOnlineUser ", accountid, check_channel_id))
                                return true
                        end
                end
        end

        for i, channel_id in pairs(ret) do
                redis_api.cmd('statistic', accountid, 'sadd',string.format("%s:channelOnlineUser",channel_id),accountid)
        end

end


local function remove_user_from_follow_channellist(accountid)                                                                    
        if not accountid  or #tostring(accountid) ~= 10 then return end
        local ok, ret = redis_api.cmd('private', accountid, 'smembers', string.format("%s:userFollowMicroChannel",accountid))
        if not ok or not ret or #ret < 1 then
                return
        end

        for i, channel_id in pairs(ret) do
                redis_api.cmd('statistic', accountid, 'srem',string.format("%s:channelOnlineUser",channel_id),accountid )
        end
end
-- 判断开关机信号
local function is_online (args)
    	if not args or not next(args)then 
	    	return true 
	else 
	    	if args['powerOff'] then 
		    	return false 
		else 
    			return true   
		end 
	end 
end

local function checkUserIsOffline(accountid,flags)
    return true
    ---- remove code by jiang z.s. 2015-06-24
end

function handle () 

	local req_body = pool['OUR_BODY_TABLE']	
	local status = is_online(req_body)
	checkUserIsOffline(req_body['accountID'],status)
	only.log("D","this app has finished")
end 
