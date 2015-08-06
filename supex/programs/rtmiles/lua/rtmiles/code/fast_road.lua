--版权声明：无
--文件名称：fast_road.lua
--创建者：王张彦
--创建日期：2015.4.22
--文件描述：本文件主要存放判断快车道主函数fast_car_lane,来存储GPS数据包,当数据包满足条件,
	--调用速度标准差和速度平均值函数process_speed_variance,当速度的平均速度和标准差满足条件
	--调用location_car,来判断是否有快速路
--历史记录：无
local only 			= require('only')
local cjson			= require('cjson')
local redis_api			= require('redis_pool_api')  
local socket			= require('socket')
local pool			= require('pool')
local fun_point_match_road	= require('fun_point_match_road')
local utils			= require('utils')
local weibo 			= require('weibo')
local check_gps_parameter 	= require('check_gps_parameter')

module('fast_road', package.seeall)

local PACKET_NUMBER		= 9	-->>包数阈值
local JUDGE_AVER 		= 50	-->>平均值判断标准
local JUDGE_STAND_DEVIATION	= 20	-->>标准差的判断标准      
local PACKET_AVER		= 40	-->>每个包的平均值判断标准         
local NEDS			= 2	-->>获取Road_ID的最大个数
local TIME_LIMIT		= 11	-->>包与包之间的时间限制
local FREQUENCY			= 60*9

local IMEI_TEST_LIST = {
	["199563123936856"] = true,
	["824478944758327"] = true,
	["166059317303275"] = true,
	["871107221461212"] = true,
	["639993286462218"] = true,
	["539281561163912"] = true,
	["481001458136617"] = true,
	["598424159840685"] = true
}

local lon			
local lat
local rid
local account_id
local frequency_key
-->>测试时使用
local function  send_frequency(imei, key, new_time)
--	only.log("I", "imei="..imei..";"..new_time)
	local ok_ttl,old_time = redis_api.cmd('mapDrimode', imei, 'get', key)
	if old_time == nil then
		redis_api.cmd('mapDrimode', imei, 'set', key, new_time)
		return false
	end
--	only.log("I", "imei="..imei..";"..old_time)
	local time_gap = new_time - old_time
	if time_gap > FREQUENCY then
--		only.log("I", "key")
		redis_api.cmd('mapDrimode', imei, 'set', key, new_time)
		return false 
	end
	return true 
end

-->>测试时使用
local function send_msg_weibo(imei, rt, gj, gps_time)	
	local fileURL = "http://127.0.0.1/productList/identification/"
	local voice
	if rt == '10' then
		if gj == '1' then
			fileURL = fileURL.."elevated_road.amr"
			voice   = "gj="..gj..";elevated_road.amr"
		elseif gj == '0' then
			fileURL	= fileURL.."fast_road.amr"
			voice	= "gj="..gj..";fast_road.amr"
		end
	elseif rt == '0' then
		fileURL	= fileURL.."high_way.amr"
		voice   = "high_way.amr"	
		local ok_send = send_frequency(imei, frequency_key, gps_time)
		if  ok_send then
			return true
		end		
	end
	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = account_id,
		level = 1,
		interval = 60*10,
		senderType = 2,
		}
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	only.log("I", "imei="..imei..":weibo send".."gpsTime:"..gps_time)
	local ok,err = weibo.send_weibo( server, "personal", wb, "2694681420",
					 "390F722AA55C96C83D76A08BA97F0E1B8846C095" )
	if not ok then
		only.log("I", "f:经度="..lon..";纬度="..lat..";Road_Id ="..rid..";imei ="..imei..
				";account_ID="..account_id..";RT="..rt..";voice="..voice..";"..err)
		return false
	end
	only.log("I", "s:经度="..lon..";纬度="..lat..";Road_Id ="..rid..
		";imei ="..imei..";account_ID="..account_id..";RT="..rt..";voice="..voice)
	return true
end

--名称：location_car
--功能：获取当前道路的状况,是否有快车道.
--参数：gps_packet,GPS数据包;
--返回值：如果是快车道返回true,否则返回false
--修改：新生成函数,王张彦,2015.4.28
local function  location_car(gps_packet, imei )	
	local direction_array	=	gps_packet["direction"]		-->>取出方向角
	local longitude_array	=	gps_packet["longitude"]		-->>取出经度	
	local latitude_array	=	gps_packet["latitude"]		-->>取出维度    
	local point_number	=	#gps_packet["GPSTime"]		-->>长度
	local gps_time		=	gps_packet["GPSTime"][1]
	for k = 1, point_number/2, 5 do
		-->>计算当前道路的road_id，可能是几条 
		local entry_ok ,fast_lane = fun_point_match_road.entry(	direction_array[k], 
									longitude_array[k],
									latitude_array[k], 
									nil, NEDS )
		--only.log('D', scan.dump(fast_lane))
		if not entry_ok  then
			only.log('E', "entry_ok  error!")
			return false
		end
		for key_lan = 1, #fast_lane do
			-->>道路信息的键值
			key = fast_lane[key_lan]["roadID"] .. ":roadInfo"
			-->>获取道路的所拥有的信息
			local ok_rt, kv_tab = redis_api.cmd('mapRoadInfo', imei, 'hgetall', key)
			--only.log('D', scan.dump(kv_tab))
			if not ok_rt then
				only.log('E', "ok_rt  error!")
				return false
			end

			lon = longitude_array[k]		-->>测试时输出经度
			lat = latitude_array[k]			-->>测试时输出维度
			rid = fast_lane[key_lan]["roadID"]	-->>测试时输出roadID

			if kv_tab['RT'] == '10'or kv_tab['RT'] == '0' then
				-->>测试时使用
				local rt = kv_tab['RT']
				-->>测试时使用
				local gj = kv_tab['GJ']
				-->>测试时使用
				send_msg_weibo(imei, rt, gj, gps_time)
				return true
			end	
		end
	end
	return false
end

--名称：process_speed_variance
--功能：计算速度的平均值和标准差,判断地理位置
--参数：new_one_gps,GPS新的数据包;fifo_key,存储数据的redis键
--返回值：是快车道返回true,否则返回false
--修改：新生成函数,王张彦,2015.4.28
local function process_speed_variance( imei, new_one_gps, fifo_key, json_string)
	local all_gps_packet = new_one_gps
	-->>取出redis 里面的所有的GPS数据
	local ok,all_redis_packet = redis_api.cmd('mapDrimode', imei, 'lrange', fifo_key, 0, -1)
	for i = #all_redis_packet , 1 , -1 do
		-->对每个字符串进行解码
		local ok,old_gps_packet = pcall(cjson.decode, all_redis_packet[i])
		if not ok then
			only.log('E', "get all packet json_encode error!")
			return false
		end
		local tmp = old_gps_packet
		for j=1,#tmp["GPSTime"] do
			-->把每个数据插入相应的数组内,倒序的插入
			table.insert(all_gps_packet["direction"], tmp["direction"][j])
			table.insert(all_gps_packet["longitude"], tmp["longitude"][j])
			table.insert(all_gps_packet["latitude"], tmp["latitude"][j])
			table.insert(all_gps_packet["GPSTime"], tmp["GPSTime"][j])
			table.insert(all_gps_packet["speed"], tmp["speed"][j])
		end
	end
	-->>取出速度数组	
	local speed_array	= all_gps_packet["speed"]
	local speed_number	= #speed_array
	local sum		= 0
	-->>计算平均速度
	for _, v in ipairs(speed_array) do
		sum = sum + v
	end
	local speed_aver = sum / speed_number
	if speed_aver < JUDGE_AVER then
		-->>,删除redis里面的第一个数据包
		redis_api.cmd('mapDrimode', imei, 'lpop', fifo_key)
		-->>新的数据包存入redis里面
		redis_api.cmd('mapDrimode', imei, 'rpush', fifo_key, json_string)
		return false
	end
	-->>计算标准差
	local variance = 0
	for _, v in ipairs(speed_array) do
		local temp = speed_aver - v
		variance = variance + math.pow(temp, 2)
	end
	local speed_variance		= variance / speed_number
	local standard_deviation	= math.sqrt(speed_variance)
	if standard_deviation > JUDGE_STAND_DEVIATION then
		-->>,删除redis里面的第一个数据包
		redis_api.cmd('mapDrimode', imei, 'lpop', fifo_key)
		-->>新的数据包存入redis里面
		redis_api.cmd('mapDrimode', imei, 'rpush', fifo_key, json_string)
		return false
	end
	--only.log('I', "speed  ok")
	-->>判断车的地理位置	
	local ok_fast_lane = location_car(all_gps_packet, imei)
	if not ok_fast_lane then
		-->>附近没有快车道，清空redis里面的数据
		redis_api.cmd('mapDrimode', imei, 'del', fifo_key)
		only.log('I', "near no fast or high, clear imei="..imei..":redis all packet")
		return false
	end
	--only.log('I', string.format("aver= %f ;devi= %f", speed_aver,standard_deviation))
	return true
end

--名称：packet_interval_time
--功能：计算新的数据包与以前的数据包的时间间隔
--参数：new_one_gps,GPS新的数据包;fifo_key,存储数据的redis键
	--json_sting,GPS数据包转码后的字符差
--返回值：是快车道返回true,否则返回false
--修改：新生成函数,王张彦,2015.4.28
local function  packet_interval_time( imei, new_one_gps, fifo_key, json_string)
	-->>从redis里面取出最后一个字符串
	local ok,redis_last_packet = redis_api.cmd('mapDrimode', imei, 'lrange', fifo_key, -1, -1)
	-->>进行解码
	local ok,old_last_packet = pcall( cjson.decode, redis_last_packet[1])
	if not ok then
		only.log('E', "json_encode error!")
		return false
	end
	-->>计算时间差
	local k = #new_one_gps['GPSTime']
	local diff_time = new_one_gps['GPSTime'][k] - old_last_packet['GPSTime'][1]
	if diff_time > TIME_LIMIT then
		-->>删除redis里面所有的数据	
		only.log('I', "time gape long ,clear imei="..imei..":all packet")
		redis_api.cmd('mapDrimode', imei, 'del', fifo_key)
		-->>新的数据存入redis
		redis_api.cmd('mapDrimode', imei, 'rpush', fifo_key, json_string)
		return false		
	end	
	return true
end

--名称：fast_car_lane
--功能：存入GPS数据包,满足条件对数据进行处理
--返回值：快车道返回true,否则返回false
--修改：新生成函数,王张彦,2015.5.7
local function  fast_car_lane( imei, new_one_gps, fifo_key)
	-->> 编码新的gps
	local ok,json_string = pcall(cjson.encode, new_one_gps)
	if not ok then
		only.log('E', "json_encode error!")
		return false
	end
	-->>redis里面数据包的个数
	local ok, len = redis_api.cmd('mapDrimode', imei, 'llen', fifo_key)
	if len < PACKET_NUMBER - 1 then
		if len == 0 then
			-->>redis没有数据，可以直接存入
			redis_api.cmd('mapDrimode', imei, 'rpush', fifo_key, json_string)
			return false
		end
			-->>检查新的数据包与以前的数据包的时间间隔
			local ok = packet_interval_time( imei, new_one_gps, fifo_key, json_string)
			if not ok then
				return false
			end
			-->>新的数据包与以前的数据包时间间隔满足条件,存入redis
			redis_api.cmd('mapDrimode', imei, 'rpush', fifo_key, json_string)
			return false		
	end
		-->>redis里面的数据包个数满足条件，检查新包与最后一个包的时间间隔
		local ok = packet_interval_time( imei, new_one_gps, fifo_key, json_string)
		if not ok then
			return false
		end
		-->>调用标准差处理函数,判断是不是快车道
		local ok = process_speed_variance(imei, new_one_gps, fifo_key, json_string)	
		if not ok then
			return false
		end
		-->>确认是快车道,删除redis里面的所有的数据
		redis_api.cmd('mapDrimode', imei, 'del', fifo_key)
	return true		
end

--名称：handle
--功能：程序的入口函数
--参数：无
--返回值：快车道返回true
--修改：新生成函数,王张彦,2015.4.22
function handle()
	local body	= pool["OUR_BODY_TABLE"]	-->>取得一个GPS数据包
	local imei	= body['IMEI']							
	account_id	= body['accountID']					
	-->> 检查参数
	local ok = check_gps_parameter.check_parameter(body)
	if not ok then
		return false
	end
	-->> 测试帐号
	if not IMEI_TEST_LIST[ imei ] then
		return false
	end
	only.log("I", "imei="..imei..":affirm")
	-->> 求每个GPS数据包的速度平均值
	local speed_array	= body['speed']			-->>取速度数据
	local sum = 0
	for _,v in ipairs(speed_array) do
		sum = sum + v
	end 
	local point_number		= #body['GPSTime']			-->>一个GPS数据包中含有的GPS点数
	local speed_aver 		= sum / point_number
	local fifo_key			= imei..":fastRoadPackets"		-->>存储数据redis的键值
	local gps_time			= body['GPSTime'][1]			-->>测试使用
	frequency_key			= imei..":highWayWeiBoTimeGap"		-->>测试使用
	-->> 过滤速度不满足的情况
	if speed_aver < PACKET_AVER then
		-->>测试时使用
		redis_api.cmd('mapDrimode', imei, 'del', frequency_key)
		-->>测试时使用
		only.log("I", "clear:imei="..imei.." :time key")
		-->>速度平均值过小,删除redis里面的数据
		redis_api.cmd('mapDrimode', imei, 'del', fifo_key)
		only.log("I", "packet speed more litter clear imei ="..imei..":redis")
		return false
	end
	-->>把方向角，经度，维度，速度，时间打包
	local new_gps  = {
		['direction']	= body['direction'],
		['longitude']	= body['longitude'],
		['latitude']	= body['latitude'],
		['speed']	= body['speed'],
		['GPSTime']	= body['GPSTime']
	}
	-->>调用数据处理函数，来判断快车道
	local ok_fast = fast_car_lane( imei, new_gps, fifo_key)
	if not ok_fast then
		return false
	end
	return true 
end
