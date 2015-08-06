--版权声明：暂无
--文件名：viaduct_rec.lua
--创建者：滕兴惠
--创建时间：2015/5/10
--文件描述：进行高架的识别
--历史记录：无

local pool	       		= require('pool')
local socket			= require('socket')
local cjson			= require('cjson')
local redis_api			= require('redis_pool_api')
local fun_point_match_road	= require('fun_point_match_road')
local utils			= require('utils')
local weibo			= require('weibo')
local link			= require('link')
local check_gps_parameter	= require('check_gps_parameter')


local NEEDS			= 2   	--需要返回roadid的个数
local ALT_UP_LIMIT		= 10	--海拔上升的界限
local ALT_DOWN_LIMIT		= -10	--海拔下降的界限
local UPHILL			= '2'	--上坡
local DOWNHILL			= '1'	--下坡
local EXIT_RAMP			= '1'	--出口匝道
local ON_RAMP			= '2'	--人口匝道
local TIME_LIMIT		= 15	--计算海拔趋势的时间限制

local IMEI_TEST_LIST = {
	["199563123936856"] = true,
	["824478944758327"] = true,
	["166059317303275"] = true,
	["871107221461212"] = true,
	["639993286462218"] = true,
	["539281561163912"] = true,
	["598424159840685"] = true,
	["481001458136617"] = true
}

local logt
local lat
local time

module('viaduct_rec',package.seeall)

--名称：get_updownhill
--功能：得到上下坡信息
--参数：海拔趋势
--返回值：上下坡信息或false-未得到信息
function get_updownhill(alt_trend)
	local updownhill_flag = 0
	if alt_trend >= ALT_UP_LIMIT then
		updownhill_flag = UPHILL
	elseif alt_trend <= ALT_DOWN_LIMIT then
		updownhill_flag = DOWNHILL
	else
		return false
	end
	return updownhill_flag
end

--名称：get_RI
--功能：获得匝道信息
--参数：imei，经度，纬度，方向角
--返回值：匝道信息或false-获取失败 
function  get_RI(imei,longitude,latitude,direction,gpstime)	
	for i = 1,#longitude do
		local ok,road_id = fun_point_match_road.entry(direction[i],longitude[i],latitude[i],nil,NEEDS)
		if not ok then
			only.log('I',"return roadID failed")
			return false
		end
		for key,_ in pairs(road_id) do
			local rdkey =  road_id[key]['roadID'] .. ":roadInfo"
			-->>得到匝道到信息
			local ok,kv_tab = redis_api.cmd('mapRoadInfo', imei , 'hmget', rdkey,'RI')
			only.log('D',scan.dump(kv_tab))
			if not ok then
				only.log('E',"get RI information failed")
				return false
			end
			if kv_tab[1] == ON_RAMP or kv_tab[1] == EXIT_RAMP then
				logt	= longitude[i]
				lat	= latitude[i]
				time 	= gpstime[i]
				return kv_tab[1]
			end				
		end
	end
	return false
end


--名称：send_onviaduct_weibo
--功能：发送是否在高架上到微博给用户
--参数：imei、状态值
--返回值：无
function send_onviaduct_weibo(accountID,idx)
	
	local fileURL = "http://127.0.0.1/productList/identification/" .. idx
	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = 1,
		interval = 60 * 10,
		senderType = 2
	}	
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	only.log('I',"call send_weibo function")
	local ok,err = weibo.send_weibo( server, "personal", wb, "4243606574", "D33D613665B352AB62E4B6806638EB1E951E76E4" )
	if not ok then
		only.log("E", "send weibo failed : " .. err)
	end
end


--名称：judge_viaduct_status
--功能：判断是否正在上下高架
--参数：imei,road_id,上下坡标志
--返回值：true-判断成功 false-判断不成功

function judge_viaduct_status(imei,accountID,updownhill_flag,RI,start_time)
	if not RI then
		if updownhill_flag == UPHILL then
			only.log('I',string.format(">>>>%s uphill call send_weibo>>开始时间:%s>>结束时间：%s>>经度：%s>>纬度：%s",imei,start_time,time,logt,lat))
--			send_onviaduct_weibo(accountID,"uphill.amr")
			only.log('I',string.format(">>>>%s uphill send_weibo success",imei))
--			print("正在上坡")
		else
			only.log('I',string.format(">>>>%s downhill call send_weibo>>开始时间:%s>>结束时间：%s>>经度：%s>>纬度：%s",imei,start_time,time,logt,lat))
--			send_onviaduct_weibo(accountID,"downhill.amr")
			only.log('I',string.format(">>>>%s downhill send_weibo success",imei))
--			print("正在下坡")
		end
		return true			
	end
	-->>判断是上匝道，还是在下匝道
	if updownhill_flag == UPHILL and RI == ON_RAMP then 
		redis_api.cmd('mapDrimode',imei,'set',imei .. ":OnViaduct",'yes')
--		print(imei,"正在上高架")
		only.log('I',string.format(">>>>%s upviaduct call send_weibo>>开始时间:%s>>结束时间：%s>>经度：%s>>纬度：%s",imei,start_time,time,logt,lat))
--		send_onviaduct_weibo(accountID,"upviaduct.amr")
		only.log('I',string.format(">>>>%s send_weibo success",imei))	
	elseif updownhill_flag == DOWNHILL and RI == EXIT_RAMP then 
		redis_api.cmd('mapDrimode',imei,'set',imei .. ":OnViaduct",'no')
--		print(imei,"正在下高架")
		only.log('I',string.format(">>>>%s downviaduct call send_weibo>>开始时间:%s>>结束时间：%s>>经度：%s>>纬度：%s",imei,start_time,time,logt,lat))
--	        send_onviaduct_weibo(accountID,"downviaduct.amr")
		only.log('I',string.format(">>>>%s send_weibo success",imei))	
	else
		return false
	end
	return true
end
--名称：process_data
--功能：对旧的海拔趋势和新的海拔趋势一起进行数据处理
--参数：imei，accountID，经度，纬度，方向角，海拔，旧的打包数据，新的打包数据，redis键值
--返回值：false-达到趋势限制，然后删除redis中的键值
function process_data(imei,accountID,longitude,latitude,direction,altitude,gpstime,old_tab,new_tab,viaduct_data)
	local link_packet_alt = altitude[#altitude] - old_tab["last_alt"]
	local val = old_tab["alt_updown_trend"] + new_tab["alt_updown_trend"] + link_packet_alt
	local updownhill_flag = get_updownhill(val)
	if not updownhill_flag then
		local new_fmt_data = pack_json(old_tab["start_time"],val,altitude[1],old_tab["TokenCode"],old_tab["flag"])
		redis_api.cmd('mapDrimode', imei, 'set',viaduct_data,new_fmt_data)
	else
		local ri = get_RI(imei,longitude,latitude,direction,gpstime)
		judge_viaduct_status(imei,accountID,updownhill_flag,ri,old_tab["start_time"],time)
		local new_fmt_data = pack_json(0,0,0,old_tab["TokenCode"],old_tab["flag"])
		redis_api.cmd('mapDrimode', imei, 'set',viaduct_data,new_fmt_data)
		return false
	end
end

--名称：pack_json
--功能：对数据进行打包转码
--参数：gpstime、海拔变化趋势、上个包的最新海拔
--返回值：转码后的字符串
function pack_json(start_time,alt_trend,last_alt,tokencode,flag)
	local new_tab = {
		["start_time"] = start_time, 
		["alt_updown_trend"] = alt_trend,
		["last_alt"] = last_alt,
		["TokenCode"] = tokencode,
		["flag"] = flag
	}
	local ok,new_fmt_data = pcall(cjson.encode,new_tab)
	return new_fmt_data
end

--名称：check_time_out
--功能：检查是否超时
--参数：imei，键值，旧的打包数据，新的打包数据
--返回值：无
function check_time_out(imei,viaduct_data,old_tab,end_time)
	local interval_time = end_time - old_tab["start_time"]
	if interval_time > TIME_LIMIT then
		only.log('I',"time is over.....")
		local new_fmt_data = pack_json(0,0,0,old_tab["TokenCode"],old_tab["flag"])
		redis_api.cmd('mapDrimode', imei, 'set',viaduct_data,new_fmt_data)
		return false
	end
end


function handle()
	local req_body		= pool["OUR_BODY_TABLE"]
	local imei		= req_body["IMEI"]
	local accountID		= req_body["accountID"]
	local direction 	= req_body['direction']
	local longitude		= req_body['longitude']
	local latitude		= req_body['latitude']
	local altitude		= req_body['altitude']
	local speed		= req_body['speed']
	local gpstime	  	= req_body['GPSTime']
	local tokencode		= req_body['tokenCode']
	-->>限定为内部的imei
	if not IMEI_TEST_LIST[imei] then
		return false
	end

	-->> 参数检测
	local ok = check_gps_parameter.check_parameter(req_body)
	if not ok then
		only.log('E','check parameter failed')
		return false
	end
	
	logt = longitude[1]
	lat = latitude[1]
	time = gpstime[1]

	-->>redis中存储的键值
	local viaduct_data = imei .. ":viaduct_data"
	-->>判断数据包的海拔趋势
	local alt_trend = 0
	for i = #altitude,2,-1 do
		alt_trend = alt_trend + (tonumber(altitude[i-1]) - tonumber(altitude[i]))
	end
	local new_fmt_data = '0'
--[[
	-->>如果一个包的数据满足上下坡趋势的变化,直接判断并修正相关的数据
	local updownhill_flag = get_updownhill(alt_trend)
	if updownhill_flag then
--		print("满足单个包")
		only.log("满足单个包")
		local ri = get_RI(imei,longitude,latitude,direction)
		judge_viaduct_status(imei,accountID,updownhill_flag,ri,gpstime[#gpstime])
		redis_api.cmd('mapDrimode', imei, 'del',viaduct_data)
		only.log('I',"single packet has met demands")
		return false
	end
--]]
	-->>从新的数据包提取数据并打包
	local new_tab = {
		["start_time"] = gpstime[#gpstime], 
		["alt_updown_trend"] = alt_trend,
		["last_alt"] = altitude[1],
		["TokenCode"] = tokencode,
		["flag"] = 0
	}
		--only.log('D',scan.dump(new_tab))

	-->>将新的数据包转码
	local ok,new_fmt_data = pcall(cjson.encode,new_tab)
 	-->>取出老的数据参与计算
	local ok,old_fmt_data = redis_api.cmd('mapDrimode', imei, 'get',viaduct_data)
	if not ok then
		only.log('E',"oprate redis failure")
		return false
	end
	-->>如果没有取到，则直接进行设置
	if not old_fmt_data then
		only.log('E',"value is nil")
		redis_api.cmd('mapDrimode', imei, 'set',viaduct_data,new_fmt_data)	
		return false
	end
	-->>对老的数据进行json解码
	local ok,old_tab = pcall(cjson.decode,old_fmt_data)
	only.log('D',scan.dump(old_tab))
	if not ok then
		only.log('E',"decode data failed")
	end
	-->>比较两个数据包的tokencode是否相同
	if new_tab["TokenCode"] ~= old_tab["TokenCode"] then
		local flag = 1
		local new_fmt_data = pack_json(0,0,0,new_tab["TokenCode"],flag)
		redis_api.cmd('mapDrimode', imei, 'set',viaduct_data,new_fmt_data)
		return false
	end
	if old_tab["flag"] < 4 then 
		local flag = old_tab["flag"] + 1
		print(old_tab["flag"])
		local new_fmt_data = pack_json(0,0,0,new_tab["TokenCode"],flag)
		redis_api.cmd('mapDrimode', imei, 'set',viaduct_data,new_fmt_data)
		return false
	end
	-->>数据包的gpstime是否为零
	if old_tab["start_time"] == '0' then
		redis_api.cmd('mapDrimode', imei, 'set',viaduct_data,new_fmt_data)
		return false
	end
	
	-->>如果变化趋势为零，接受下一个数据包
	if alt_trend == 0 then
		check_time_out(imei,viaduct_data,old_tab,gpstime[1])
		return false
	end
	-->>新的数据包和老的数据联合进行计算
	if (old_tab["alt_updown_trend"] > 0 and new_tab["alt_updown_trend"] > 0) or
		(old_tab["alt_updown_trend"] < 0 and new_tab["alt_updown_trend"] < 0) then
		process_data(imei,accountID,longitude,latitude,direction,altitude,gpstime,old_tab,new_tab,viaduct_data)
	else
		-->>如果新数据包中海拔的上升趋势和以前的上升趋势不是同一趋势，去除小幅波动
		if(math.abs(old_tab["alt_updown_trend"]) - math.abs(new_tab["alt_updown_trend"])) > 0 then
			process_data(imei,accountID,longitude,latitude,direction,altitude,gpstime,old_tab,new_tab,viaduct_data)
		elseif (math.abs(old_tab["alt_updown_trend"]) - math.abs(new_tab["alt_updown_trend"])) == 0 then
			local new_fmt_data = pack_json(gpstime[1],0,altitude[1],old_tab["TokenCode"],old_tab["flag"])
			redis_api.cmd('mapDrimode', imei, 'set',viaduct_data,new_fmt_data)
			return true
		else
			local new_fmt_data = pack_json(gpstime[#gpstime],alt_trend,altitude[1],old_tab["TokenCode"],old_tab["flag"])
			redis_api.cmd('mapDrimode', imei, 'set',viaduct_data,new_fmt_data)
			return true
		end
	end
	-->>判断是否超出了时间限制
	check_time_out(imei,viaduct_data,old_tab,gpstime[1])
	return true
end
