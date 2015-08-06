--
-- 版权声明: 暂无
-- 文件名称: alone_utils.lua
-- 创建者  :  
-- 创建日期: 
-- 文件描述: 本文件存放在appcenter项目alone模式下的应用中用到的公共函数
-- 历史记录: 无
--
local APP_POOL           = require("pool")
local APP_UTILS          = require("utils")
local APP_ONLY           = require("only")
local APP_CFG            = require("cfg")
local APP_CONFIG_LIST    = require("CONFIG_LIST")
local APP_BOOL_FUNC_LIST = require("BOOL_FUNC_LIST")
local APP_WORK_FUNC_LIST = require("WORK_FUNC_LIST")
local link               = require("link")
local redis_api          = require("redis_pool_api")
local supex              = require('supex')
local zlib               = require("zlib")


module("alone_utils", package.seeall)

local spx_txt_to_voice  = link["OWN_DIED"]["http"]["spx_txt_to_voice"]
local personalWeibo = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]

local split_str                = 'abc_appcenter_really_send_msg_to_weibo_test_2014_test_really'
local data_multi_personal_head = 'POST /weiboapi/v2/sendMultimediaPersonalWeibo HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\nContent-Type:content-type:multipart/form-data;boundary=' .. split_str .. '\r\n\r\n%s'
local data_splitend            = "--" .. split_str .. "--"
local data_format_normal       = '--' .. split_str .. '\r\nContent-Disposition:form-data;name="%s"\r\n\r\n%s\r\n'

local mapapi_getneardaoke_path = 'mapapi/v2/getNearbyDaoke'


local post_format = 'POST /%s HTTP/1.0\r\n' ..
'Host:%s:%s\r\n' ..
'Content-Length:%d\r\n' ..
'Content-Type:application/json\r\n\r\n%s'

---- 发送个人多媒体微博 
function really_send_multi_personal_weibo(wb)
	local data_tab = {}
	for i,v in pairs(wb) do
		table.insert(data_tab,string.format(data_format_normal,i,v))
	end
	local host_name = personalWeibo.host
	local host_port = personalWeibo.port
	local data = string.format("%s%s",table.concat( data_tab, "") , data_splitend )
	local req = string.format(data_multi_personal_head,host_name,host_port,#data,data)
	return utils.really_send_multi_personal_weibo(host_name,host_port,req)
end

function txt_2_voice(appKey, secret, text)
	local tmp_txt_crc32  = zlib.crc32()(text)
	local ok,tmp_reids_url = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'hmget',"NormalTextURLHash",string.format("%d:fileURL",tmp_txt_crc32), string.format("%d:fileID",tmp_txt_crc32) )
	if ok and tmp_reids_url and #tmp_reids_url == 2 then
		only.log('D',string.format("txt_2_voice: %s url:%s from redis--->---",text,tmp_reids_url[1]))
		return true,tmp_reids_url[1],tmp_reids_url[2]
	end
	local status, file_url,file_id = utils.txt_2_voice( spx_txt_to_voice, appKey , secret,text )
	if status then
		redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'hmset',"NormalTextURLHash",string.format("%d:fileURL",tmp_txt_crc32), file_url , string.format("%d:fileID",tmp_txt_crc32) , file_id )
	end

	only.log('D',string.format("%s  txt_2_voice: %s url:%s from api--->---",status ,text,file_url))
	return status, file_url,file_id
end

--获取在线用户总数
function get_onlineuser_count()
	local ok_status,ok_count = redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"],'scard','onlineUser:set')
	if not ok_status then
		return 0
	end
	return tonumber(ok_count) or 0
end

--获取所有在线用户
function get_onlineuser_tab()
	local ok_status,ok_tab = redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"],'smembers','onlineUser:set')
	if not ok_status or ok_tab == nil then
		return nil
	end
	return ok_tab
end

--随机获取X个在线用户
function get_onlineuser_by_random(icount)
	if not icount then return nil end
	if icount < 1 then return nil end
	local ok_status,ok_tab = redis_api.cmd('statistic',APP_POOL["OUR_BODY_TABLE"]["accountID"],'srandmember','onlineUser:set',icount)
	if not ok_status or ok_tab == nil then return nil end
	return ok_tab 
end



function get_around_daoke(appKey, secret, longitude, latitude, distance  )
	----test code
	--[[
	221.228.231.84:80/getNearbyDaoke 
	参数传递格式：longitude=121.3605&latitude=31.224&distance=12000&sign=BD99C86ED029E2AF49C8716FC424D875521373CE&appKey=2064302565 
	]]

	if tonumber(longitude) == 0  or tonumber(latitude) == 0 then
		only.log('D','get_around_daoke longitude or latitude is 0 ')
		return false,nil
	end

	local args = {
		appKey    = appKey,
		longitude = longitude,
		latitude  = latitude,
		distance  = distance,
	}
	---- add new 
	args["sign"]      = utils.gen_sign(args, secret)
	local body        = utils.table_to_kv(args)
	local req = string.format(post_format, mapapi_getneardaoke_path ,mapAroundDaoke.host, tostring(mapAroundDaoke.port), #body ,body )
	local ok,ret = supex.http(mapAroundDaoke.host, mapAroundDaoke.port, req, #req)
	if not ok then
		only.log('E',string.format("[FAILED] %s %s HTTP post failed! get_around_daoke api ", mapAroundDaoke.host, mapAroundDaoke.port ) )
		return false,nil
	end

	only.log('D',ret)

	local ret_str = string.match(ret,'{.+}')
	if not ret_str then
		return false,nil
	end

	local ok_status,ok_tab = pcall(cjson.decode , ret_str)
	if not ok_status then
		return false,nil
	end

	if ok_tab['ERRORCODE'] ~= "0" then
		return false, nil
	end

	local detail_tab = ok_tab['RESULT']
	if type(detail_tab) ~= "table" or #detail_tab < 1 then
		return false ,nil
	end

	local ret_tab = {}
	for i,v in pairs(detail_tab) do
		if v and v['accountID'] ~= nil then
			ret_tab[ v['accountID'] ] =   v['accountID']
		end
	end
	--TODO 
	return true , ret_tab
end

function get_last_timestamp(accountid)
	----获取最新GPS坐标对应的时间
	local ok_status,ok_timestamp  = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get',accountid .. ':currentGPSTime')
	if not ok_status or ok_timestamp == nil then
		return 0
	end
	return ok_timestamp or 0
end

---- return
---- 1 true/false
---- 2 longitude
---- 3 latitude
function get_point(accountid)
	local ok_timestamp = get_last_timestamp(accountid)
	if os.time() -  ok_timestamp > 5*60 then return false,nil,nil end

	local ok,gps_str = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', api_data["accountID"] .. ":currentBL")
	if ok and gps_str then
		local gps_info = utils.str_split(gps_str,",")
		local longitude = gps_info[1]
		local latitude  = gps_info[2]
		return true,longitude,latitude
	end

	return false,nil,nil

end


---- 通过accountid 获取用户的nickname
function get_nickname(accountid)
	if not accountid then return false,nil end
	local ok_status,ok_nickname = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', accountid .. ':nickname')
	if not ok_status then
		return false,nil
	end
	return true,ok_nickname
end

function get_imei_by_accountid(accountid)
	if not accountid then return false,nil end
	local ok_status,ok_imei = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', accountid .. ':IMEI')
	if not ok_status then
		return false,nil
	end
	return true,ok_imei
end

function number_to_chinese(num)
	if not num then return '' end
	if type(tonumber(num)) ~= "number" then return '' end
	local num_list = {[0] = '零',[1] = '一',[2] = '二',[3] = '三',[4] = '四',[5] = '五',[6] = '六',[7] = '七',[8] = '八',[9] = '九'}
	local str = ''
	for i = 1, #num do
		str = str .. num_list[tonumber(string.sub(num,i,i))]
	end
	return str
end


function get_daokenum_by_imei(imei)
	if not imei then return '' end
	if #tostring(imei) ~= 15 then  return '' end
	return number_to_chinese( string.sub(imei,12,#imei) )
end
