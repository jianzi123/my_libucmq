--
-- 版权声明: 暂无
-- 文件名称: four_miles_ahead.lua
-- 创 建 者: yy
-- 创建日期: 2015/06/26
-- 文件描述: 前方4公里主处理逻辑以及其他辅助函数
-- 历史记录: 无
--
local redis_api		= require('redis_pool_api')
local APP_CONFIG_LIST	= require('CONFIG_LIST')
local APP_POOL		= require('pool')
local cjson		= require('cjson')
local link		= require('link')
local cutils		= require('cutils')
local supex		= require('supex')
local utils		= require('utils')
local only		= require('only')
local http_short	= require('http_short_api')
local weibo		= require("weibo")
local init_data		= require("init_data")
local four_miles_sence = require("four_miles_sence")
module(..., package.seeall)

--默认POI下发规则列表
local POI_LIST = {
	["1118102"] = {txt = "动物园", dis=500, interval = 30, receiver_dis=150, context = "动物园"},
	["1118103"] = {txt = "植物园", dis=500, interval = 30, receiver_dis=150, context = "植物园"},
	["1122106"] = {txt = "驾校", dis=500, interval = 30, receiver_dis=150, context = "附近有驾校，请注意马路杀手"},
	["1122102"] = {txt = "中学", dis=500, interval = 30, receiver_dis=100, context = "前方是"},
	["1122103"] = {txt = "小学", dis=500, interval = 30, receiver_dis=100, context = "前方是"},
	["1122104"] = {txt = "幼儿园", dis=500, interval = 30, receiver_dis=100, context = "前方是"},
	["1114101"] = {txt = "农贸市场", dis=500, interval = 15, receiver_dis=100, context = "农贸市场路段，请缓行"},
	["1123101"] = {txt = "隧道", dis=600, interval = 30, receiver_dis=300, context = "即将进入"},
	["1123107"] = {txt = "大桥", dis=500, interval = 30, receiver_dis=200, context = "正在经过"},
	["1126111"] = {txt = "事故易发地段", dis=500, interval = 30, receiver_dis=150, context = "事故易发地段，请缓行。"},
	["1126113"] = {txt = "村庄", dis=500, interval = 30, receiver_dis=150, context = "经过村庄路段，小心岔路来车。"},
	["1126116"] = {txt = "有人看管的铁路道口", dis=500, interval = 30, receiver_dis=150, context = "前方为铁路道口，请谨慎驾驶。"},
	["1126117"] = {txt = "无人看管的铁路道口", dis=500, interval = 30, receiver_dis=150, context = "前方为铁路道口，请谨慎驾驶。"},
	["1126110"] = {txt = "注意落石", dis=500, interval = 30, receiver_dis=150, context = "该路段有落石风险，请小心驾驶。"},
	["1126118"] = {txt = "道路变窄", dis=500, interval = 30, receiver_dis=150, context = "前方道路变窄，请降低车速，小心通行。"},
	["1126119"] = {txt = "向左急弯路", dis=500, interval = 30, receiver_dis=150, context = "前方有急转弯，请降低车速，小心通行。"},
	["1126120"] = {txt = "向右急弯路", dis=500, interval = 30, receiver_dis=150, context = "前方有急转弯，请降低车速，小心通行。"},
	["1126122"] = {txt = "连续弯路", dis=500, interval = 30, receiver_dis=150, context = "前方有连续急转弯，请降低车速，小心通行。"},
	["1126123"] = {txt = "左侧合流", dis=500, interval = 30, receiver_dis=200, context = "请注意并线车辆，小心驾驶注意避让。"},
	["1126124"] = {txt = "右侧合流", dis=500, interval = 30, receiver_dis=200, context = "请注意并线车辆，小心驾驶注意避让。"},
	["1126159"] = {txt = "右转红灯", dis=500, interval = 21, receiver_dis=100, context = "前方路口右转有红灯控制，请注意交通信号灯。"},
	["1126160"] = {txt = "右转避让行人", dis=500, interval = 21, receiver_dis=100, context = "前方路口如遇红灯右转，请降低车速，礼让行人。"},
	["1123111"] = {txt = "违章摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有违章摄像头"},
	["112311101"] = {txt = "违章摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有违章摄像头"},
	["112311102"] = {txt = "违章摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有违章摄像头"},
	["1123110"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头"},
	["112311030"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速30"},
	["112311035"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速35"},
	["112311040"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速40"},
	["112311045"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速45"},
	["112311050"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速50"},
	["112311055"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速55"},
	["112311060"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速60"},
	["112311070"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速70"},
	["112311080"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速80"},
	["112311090"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速90"},
	["1123110100"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速100"},
	["1123110110"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速110"},
	["1123110120"] = {txt = "限速摄像头", dis=500, interval = 30, receiver_dis=200, level = 25, context = "前方有限速摄像头,限速120"},
	["1126112"] = {txt = "易滑", dis=500, interval = 30, receiver_dis=150, context = "易滑路段，天气不好，需更加小心。"},
	["1123105"] = {txt = "加油站", dis=500, interval = 30, receiver_dis=200, context = "马上会经过一个加油站，请检查油表油量"},
	["1123112"] = {txt = "高速跨省收费站", dis=3000, interval = 30, receiver_dis=1500, level = 30, context = "依据POIID获取语音下发"},
	["1126157"] = {txt = "高速出口", dis=1000, interval = 30, receiver_dis=300, level = 30, context = "依据POIID获取语音下发"},
	["1123106"] = {txt = "服务区", dis=4000, interval = 30, receiver_dis=2000, level = 21, context = "依据POIID获取语音下发"},
	["1118101"] = {txt = "旅游景点", dis=500, interval = 30, receiver_dis=200, context = "依据POIID获取语音下发"},
	["1123114"] = {txt = "有名大桥", dis=500, interval = 30, receiver_dis=300, dir = 30, context = "依据POIID获取语音下发"},
	["1117102"] = {txt = "特殊酒店", dis=500, interval = 30, receiver_dis=300, dir = 30, context = "依据POIID获取语音下发"},
}


--
-- 名      称: filter_by_user_subscribed
-- 功      能: 用户订阅过滤
-- 参      数: 无
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--
local function  filter_by_user_subscribed()
	for poi_type, value in pairs(weibo.DRI_APP_4_MILES) do
                if POI_LIST[poi_type] and not weibo.check_driview_subscribed_msg(accountID, value["no"])  then
                        POI_LIST[poi_type] = nil
                end
        end
end


--
-- 名      称: filter_by_time_interval
-- 功      能: 时间段过滤
-- 参      数: 无
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--
local function filter_by_time_interval()
	local month = os.date('%m')
        month = tonumber(month)
        local hour = os.date('%H')
        hour = tonumber(hour)
        local week = os.date('%w')
	week = tonumber(week)

	---school
	if hour < 6 or hour > 8 and hour < 12 or 
		hour > 13 and hour < 17 or hour > 18 then
		POI_LIST["1122102"] = nil
	end
	if hour < 6 or hour > 8 and hour < 16 or 
		hour > 17 then
		POI_LIST["1122103"] = nil
	end
	if hour < 6 or hour > 8 and hour < 15 or 
		hour > 17 then
		POI_LIST["1122104"] = nil
	end
	--小学、初中、高中
	if month == 7 or month == 8 or week == 6 or week == 7 then
		POI_LIST["1122102"] = nil
		POI_LIST["1122103"] = nil
		POI_LIST["1122104"] = nil
	end

	--农贸市场
	if hour < 7 or hour > 18 then
		POI_LIST["1114101"] = nil
	end
	--if hour < 7 or  hour > 17 then
	if hour < 7 or  hour > 17 then
		--POI_LIST[1118103] = nil
	end

	--动物园植物园
	if hour < 9 or  hour > 18 then
		POI_LIST["1118102"] = nil
		POI_LIST["1118103"] = nil
	end

	--有名大桥
	if hour < 9 or hour > 19 then
		POI_LIST["1123114"] = nil
	end

	--隧道
	if hour < 7 or hour > 19 then
		POI_LIST["1123101"] = nil
	end
end








--
-- 名      称: filter_by_road_type
-- 功      能: 道路类型过滤
-- 参      数: 无
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--
local function filter_by_road_type(road_type)
	--高速出口
	if road_type and POI_LIST["1126157"] then
		--高速
		if road_type == 0 then
                        POI_LIST["1126157"]["dis"] = 1500
                        POI_LIST["1126157"]["receiver_dis"] = 1000
                end
		--高架
		if road_type == 10 then
                        POI_LIST["1126157"]["dis"] = 1000
                        POI_LIST["1126157"]["receiver_dis"] = 500
                end
	end
	
	--有名大桥
	if road_type and POI_LIST["1123114"] then
                POI_LIST["1123114"]["dis"] = 1000
                POI_LIST["1123114"]["receiver_dis"] = 600
        end

	--隧道
	if road_type and POI_LIST["1123101"] then
                POI_LIST["1123101"]["dis"] = 1000
                POI_LIST["1123101"]["receiver_dis"] = 600
        end

	--左侧合流
	if road_type and POI_LIST["1126123"] then
                POI_LIST["1126123"]["dis"] = 1000
                POI_LIST["1126123"]["receiver_dis"] = 600
        end

	--右侧合流
	if road_type and POI_LIST["1126124"] then
                POI_LIST["1126124"]["dis"] = 1000
                POI_LIST["1126124"]["receiver_dis"] = 600
        end
end

--
-- 名      称: pre_handle_issued_rules
-- 功      能: POI下发规则预处理
-- 参      数: 无
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--
local function pre_handle_issued_rules(road_type)
	--根据用户权限过滤
	filter_by_user_subscribed()
	--根据时间段过滤
	filter_by_time_interval()
	--根据道路类型过滤
	filter_by_road_type(road_type)
end

--
-- 名      称: refresh_issued_poi_type
-- 功      能: 刷新已下发POI列表
-- 参      数: 无
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--
local function refresh_issued_poi_type(poi_issued_list, other_poi_type_issued_key, other_poi_type_issued_value, roadID)
	local roadID_poiType_set = APP_POOL["OUR_BODY_TABLE"]["accountID"] .. ':roadIDPOIType'
	--local roadID_poiType_key = APP_POOL["OUR_BODY_TABLE"]["accountID"] ..':'.. roadID
	if poi_issued_list  then
                other_poi_type_issued_value = other_poi_type_issued_value or "" .. ','.. poi_issued_list
                local ok = redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"],'set', other_poi_type_issued_key, other_poi_type_issued_value)
                if ok then
                        local ok, ret = redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"],'sadd', roadID_poiType_set, roadID)
			if not ok then
				only.log("E","refresh_issued_poi_type failed|" .. tostring(roadID_poiType_set) .. "|" .. tostring(roadID))
			end
			
                        only.log('D','sadd' .. (roadID_poiType_set or ':empty').. ", value is :"..(roadID or ':empty'))
                end
        end
end


--
-- 名      称: precond_camera_list
-- 功      能: 预处理电子眼
-- 参      数: @camera_list
-- 返  回  值: @camera_distance_list
-- 修      改: 新生成函数 yy 2015/6/26
--
local function precond_camera_list(camera_list)
	local index	 = 1
	local lon	 = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
	local lat	 = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]


	local   camera_distance_list = {}
	for poi_type, jo in pairs(camera_list or {}) do
		for i = 1 , #(jo or {}) do
			local poi_lon   = jo[i]["longitude"]
			local poi_lat   = jo[i]["latitude"]
			local mileage = cutils.gps_distance(lon, lat, poi_lon, poi_lat)
			mileage = tonumber(mileage) or 0
			camera_distance_list[index] = {}
			if poi_type == "1123110" then
	--			flag = true 
                                camera_distance_list[index]['i'] = i  
			elseif poi_type == "1123111"  then
	--			flag = true
				camera_distance_list[index]['i'] = i  + 1000 
                        end
                        camera_distance_list[index]['mileage'] = mileage
                        camera_distance_list[index]['poi_type'] = poi_type
			index = index + 1
			
		end
	end
	return camera_distance_list
end

--
-- 名      称: refresh_camera_issued_list
-- 功      能: 刷新本次开机摄像头下发列表
-- 参      数: @camera_issued_table,@camera_current_issued_table,@road_safety_camera_key
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--
local function refresh_camera_issued_list(camera_issued_table, camera_current_issued_table, road_safety_camera_key)
	local camera_current_issued_len = #camera_current_issued_table  or 0
	local camera_issued_len 	= (not camera_issued_table) and 0 or #camera_issued_table
	
	local MAX_CAMERA_ISSUED_LEN = 10
	if camera_current_issued_len > 0 then
		local camera_str = nil
		for i = 1, camera_current_issued_len do
			if i == 1 then
				camera_str = camera_current_issued_table[i]
			else
				camera_str = camera_str .. "," .. camera_current_issued_table[i]
			end
		end
		local max_len = (
			(camera_current_issued_len + camera_issued_len < MAX_CAMERA_ISSUED_LEN) and
			(camera_current_issued_len + camera_issued_len) or MAX_CAMERA_ISSUED_LEN 
		)
		for i = camera_current_issued_len+1, max_len do
			camera_str = camera_str .. "," .. camera_issued_table[i-camera_current_issued_len]
		end
		local ok, ret = redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"],'set', road_safety_camera_key, camera_str)
		if not ok then
			error(string.format("refresh camera list failed|%s", tostring(ret)))
		end
	end
end

--
-- 名      称: build_message
-- 功      能: 将下发POI信息写入redis,供后续应用读取
-- 参      数: @message_info_list
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--
local function build_message(message_info_list)
	local accountID		= APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local idx_key		= accountID .. ':4MilesAheadPositionTypeSet'
        local carry_key		= accountID .. ':4MilesAheadPositionCarry'
        redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', idx_key)
        redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', carry_key)
	for poi_type, poi_info in pairs(message_info_list or {}) do
                local ok, carry_data = pcall(cjson.encode, poi_info)
		if ok then
			local ok1, ret1 = redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hset', carry_key, poi_type, carry_data)
			if not ok1 then
				error(string.format("refresh [4MilesAheadPositionCarry] failed|%s", tostring(ret1)))
			end
			local ok2, ret2 = redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', idx_key, poi_type)
			if not ok2 then
				error(string.format("refresh [4MilesAheadPositionCarry] failed|%s", tostring(ret2)))
			end
                end
        end
end

--预处理前方4公里POI集合
--返回table结构：
--ahead_poi_table = {
--	poi_type1 = {},
--	poi_type2 = {},
--	...
--	poi_typen = {},
--}
local function prepare_ahead_poi_table()
--[[
	local ahead_poi_table = nil
	local AHEAD_POI_TABLE = nil
	--集成时去掉
	local ok, temp_table = redis_api.cmd('mapFrontPosition_new',
			 APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', APP_POOL["OUR_BODY_TABLE"]["accountID"] .. ':frontPOI')
	if not ok or not temp_table then
		error(string.format("get frontPOI failed|%s", tostring(temp_table)))
	end

	local ok, AHEAD_POI_TABLE = utils.json_decode(temp_table)
	if not ok then
		error("json decode ahead_poi failed")
	end
	--集成时去掉
]]--
	local AHEAD_POI_TABLE = APP_POOL["OUR_BODY_TABLE"]["T_FRONT_POI_SET"]
	only.log("D", string.format("[Function:prepare_ahead_poi_table] get poi from redis [value:%s]", scan.dump(AHEAD_POI_TABLE)))
	only.log("D","AHEAD_POI_TABLE is |" .. cjson.encode(AHEAD_POI_TABLE))

	if AHEAD_POI_TABLE then
		ahead_poi_table = {}
		for poiID, poiValue in pairs(AHEAD_POI_TABLE) do
                	poiValue['positionID'] = poiID
               		if not ahead_poi_table[tostring(poiValue['type'])] then
                        	ahead_poi_table[tostring(poiValue['type'])] = {}
                        	table.insert(ahead_poi_table[tostring(poiValue['type'])], poiValue)
                	else
                        	table.insert(ahead_poi_table[tostring(poiValue['type'])], poiValue)
			end
                end
	end	
	return ahead_poi_table
end

--
-- 名      称: four_miles_ahead_logic
-- 功      能: 前方4公里处理逻辑（入口函数）
-- 参      数: 无
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--

function four_miles_ahead_logic()
	--TODO 	获取前方4公里POI集合
	--ahead_poi_table  

	--前方4公里包含的所有POI(TABLE)
	--[[
	local ok, ahead_poi_table = pcall(prepare_ahead_poi_table)
	if not ok then
		only.log("E","prepare_ahead_poi_table failed|" .. tostring(ahead_poi_table))
		return false
	end
	if not next(ahead_poi_table) then
		only.log("I","ahead_poi_table is null|")
		return false
	end

	--获取用户道路信息
	--local road_info = match_road()
	local road_info = init_data.point_match_road_result
	only.log("D", string.format("[Function:four_miles_ahead_logic] point match road result :%s", road_info))
	if not road_info then
		only.log("I","match user's road failed")
		return false
	end
	]]--
	local ok, ahead_poi_table = pcall(prepare_ahead_poi_table)
        if not ok then
                only.log("E","prepare_ahead_poi_table failed|" .. tostring(ahead_poi_table))
                return false
        end
        local road_info         = APP_POOL["OUR_BODY_TABLE"]["T_FRONT_ROAD_INFO"]

        only.log('D', "ahead_poi_table:" .. scan.dump(ahead_poi_table))
        only.log('D', "road_info:" .. scan.dump(road_info))

        if type(ahead_poi_table) ~= "table" or type(road_info) ~= "table" or
                not next(ahead_poi_table) or not road_info["RT"] or not road_info["roadID"] then
                only.log("D","ahead_poi_table or road_info not suitable")
                return false
        end

	local road_type = road_info["RT"]		--道路类型
	local roadID	= road_info["roadID"]		--道路ID

	only.log('D', string.format("[Function:four_miles_ahead_logic], begin deal"))
	--POI下发规则预处理
	pre_handle_issued_rules(road_type)

	--获取POI情景处理列表
	local POI_SENCE_LIST	 = four_miles_sence["POI_SENCE_LIST"]
	if not POI_SENCE_LIST then
		only.log("E","get POI_SENCE_LIST failed")
		return false
	end

	--数据池信息
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]

	--获取本次开机已下发POI类型
	local gas_station_roadID 		= "gasStationKey"
	local gas_station_type_issued_key	= accountID .. ":" .. gas_station_roadID
	local ok, gas_station_type_issued_value	= redis_api.cmd('driview', accountID, 'get', gas_station_type_issued_key)
	local gas_station_type_issued_table	= utils.str_split(gas_station_type_issued_value,",")
	local other_poi_type_issued_key		= accountID .. ':' .. roadID
	local ok, other_poi_type_issued_value	= redis_api.cmd('driview', accountID, 'get', other_poi_type_issued_key)
	local other_poi_type_issued_table	= utils.str_split(other_poi_type_issued_value,",")

	--临时变量定义
	local temp_poi_type_issued_table	= nil						--临时变量
	local camera_list			= {}					--摄像头类别POI集合
	local poi_issued_list			= nil						--已经下发的所有POI的集合
	local message_info_list			= {}						--下发信息集合
	local temp_roadID			= nil

	--POI类型处理
	poi_issued_list = other_poi_type_issued_value



--[[
	ahead_poi_table = {
		["1118102"] = {
			{
				["angle2"]=25,
				["longitude"]=121.36029,
				["latitude"]=31.22376,
				["positionID"]="P03682445",
				["type"]=1118102,
				["angle1"]=205,
			},

			{
				["angle2"]=26,
				["longitude"]=121.36079,
				["latitude"]=31.22488,
				["positionID"]="P03665845",
				["type"]=1118102,
				["angle1"]=206,
			},

			{
				["angle2"]=25,
				["longitude"]=121.36041,
				["latitude"]=31.22413,
				["positionID"]="P03688545",
				["type"]=1118102,
				["angle1"]=205,
			},
		}
	}
]]--



	only.log("D","ahead_poi_table is |" .. tostring(cjson.encode(ahead_poi_table)))
	for poi_type, poi_type_value_table in pairs(ahead_poi_table) do
		only.log("D","in pairs table")
		only.log("D","poi_type  type is |" .. tostring(poi_type) .. "|" .. tostring(type(poi_type)))
		repeat
			if POI_LIST[poi_type] then
				--摄像头
				if  poi_type == "1123110"  or  poi_type == "1123111"  then
                                	camera_list[poi_type] = poi_type_value_table 
					break
				end
				--加油站
				if poi_type == "1123105" then
					--取加油站已下发列表 TODO
					if gas_station_type_issued_value then		--加油站已经下发过,直接跳出循环
						break
					end
					temp_poi_type_issued_table = {}
					temp_roadID = gas_station_roadID
				else
					--取其他类型POI已下发列表 TODO
					temp_poi_type_issued_table = other_poi_type_issued_table
					temp_roadID = roadID	
				end

				--处理POI类型
				--检查是否符合下发条件
				local ok, match_result = pcall(POI_SENCE_LIST[poi_type]["match"], temp_poi_type_issued_table, poi_type_value_table, POI_LIST[poi_type])
				if not ok then
					only.log("E", string.format("%s match failed|%s", poi_type, match_result))
					break
				end
				--没有下发过
				if not match_result["have_issued"] then
					local ok, ret = pcall(POI_SENCE_LIST[poi_type]["work"], match_result["poi_info"], temp_roadID, poi_issued_list, message_info_list, POI_LIST[poi_type])
					if not ok then
						only.log("E", string.format("%s work failed|%s", poi_type, ret))
						break
					end
					--LUA字符串拷贝，修复BUG
					poi_issued_list = ret
				end
			end	
			only.log("D","message_info_list  inner not camera|" .. tostring(cjson.encode(message_info_list)) )
		until 1==1
	end
	
	--将已下发所有非电子眼类POI存入redis
	refresh_issued_poi_type(poi_issued_list, other_poi_type_issued_key, other_poi_type_issued_value, roadID)
	
	only.log('D', string.format("[Function:four_miles_ahead_logic], deal carmera"))
	--处理摄像头
	local road_safety_camera_key = accountID .. ':roadSafetyCamera'
	local camera_distance_list = precond_camera_list(camera_list)

	only.log("D","message_info_list not camera|" .. tostring(cjson.encode(message_info_list)) )


	local MAX_CAMERA_ISSUD_COUNT  = 3
	if #camera_distance_list > 0 then
		table.sort(camera_distance_list, function(a, b) return a.mileage < b.mileage end)
		local issue_count = #camera_distance_list < MAX_CAMERA_ISSUD_COUNT 
			and #camera_distance_list or MAX_CAMERA_ISSUD_COUNT
		--已经下发电子眼集合
		local ok, ret = redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', road_safety_camera_key)
		if not ok then
			only.log("D","get road safty camera failed")
			return false
		end
		local camera_issued_table = utils.str_split(ret,",")
		local camera_current_issued_table = {}

		for k = 1, issue_count do
			repeat
				local i = camera_distance_list[k]['i']
				local poi_type = camera_distance_list[k]['poi_type']
				local jo
				if  i > 1000 then
					i = i - 1000
				end
				jo = camera_list[poi_type]
				local ok, have_issued = pcall(POI_SENCE_LIST[poi_type]["match"], camera_issued_table, jo[i]["positionID"])		
				if not ok then
					only.log("E", string.format("%s match exec failed", poi_type))
					break
				end
				if not have_issued then
					local ok, ret = pcall(POI_SENCE_LIST[poi_type]["work"], jo[i],
						POI_LIST, road_type, message_info_list, camera_current_issued_table)
					if not ok then
						only.log("E", string.format("%s work exec failed|%s", poi_type, tostring(ret)))
						break
					end
				end
			until 1==1
			only.log("D","message_info_list  inner camera|" .. tostring(cjson.encode(message_info_list)) )
		end

		local ok, ret = pcall(refresh_camera_issued_list, camera_issued_table,
					camera_current_issued_table, road_safety_camera_key)
		if not ok then
			only.log("E","refresh_camera_issued_list failed|" .. tostring(ret))
			return false
		end
	end	--if #camera_distance_list > 0


	only.log("D","message_info_list  camera|" .. tostring(cjson.encode(message_info_list)) )

	--将下发POI数据存入redis
	if not next(message_info_list) then
		only.log("D","message_info_list is null" .. tostring(cjson.encode(message_info_list)) )
		return false
	end

	local ok, ret = pcall(build_message, message_info_list)
	if not ok then
		only.log("E","build_message failed|" .. tostring(ret))
		return false
	end
	only.log('D', string.format("[Function:four_miles_ahead_logic] deal over"))

	return true
end
