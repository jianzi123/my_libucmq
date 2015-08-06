--
-- 版权声明: 暂无
-- 文件名称: four_miles_sence.lua
-- 创 建 者: yy
-- 创建日期: 2015/06/26
-- 文件描述: 前方4公里每类POI具体匹配与处理函数
-- 历史记录: 无
--
local redis_api         = require('redis_pool_api')
local APP_CONFIG_LIST   = require('CONFIG_LIST')
local APP_POOL          = require('pool')
local cjson             = require('cjson')
local link              = require('link')
local cutils            = require('cutils')
local supex             = require('supex')
local utils             = require('utils')
local only              = require('only')
local http_short        = require('http_short_api')
local weibo             = require("weibo")
local init_data         = require("init_data")
module(..., package.seeall)

--
-- 名      称: common_match
-- 功      能: 公共模块
-- 参      数: @poi_type,@poi_type_issued_table,@poi_type_value_table,@poi_type_issued_rules
-- 返  回  值: {'是否已下发','满足下发条件的POI信息'}
-- 修      改: 新生成函数 yy 2015/6/26
--
local function common_match(poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
	--判断该类型POI是否在当前道路上下发过
	only.log("D","in common_match" .. poi_type)
	for _, v in pairs(poi_type_issued_table or {}) do
		if poi_type == v then
			return { have_issued = true }			--已经报过
		end
	end
	
	--检查是否满足下发距离的触发条件
	local index = nil		--满足下发条件的POI索引
	for i=1,#(poi_type_value_table or {}) do
		local max_mileage = -1
		local id = poi_type_value_table[i]["positionID"]
		--可以优化成每次请求中仅get一次，set一次
		local poiID_key = APP_POOL["OUR_BODY_TABLE"]["accountID"] .. ":POIID"	--本次开机已经下发的POIID的集合
		local ok, ret = redis_api.cmd('private_hash', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sismember', poiID_key, id)
		if not ok then
			error(string.format("poi sismember operate is failed|poi_type:%s|poi_id:%s", poi_type, id))
		end
		if ret then
			return { have_issued = true }			--已经报过
		else
			local poi_lon   = poi_type_value_table[i]["longitude"]
                        local poi_lat   = poi_type_value_table[i]["latitude"]
			local lon       = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
			local lat       = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
			--计算球面距离
			local mileage = cutils.gps_distance(lon, lat, poi_lon, poi_lat)
			mileage = tonumber(mileage) or 0
			--only.log('D', 'mileage is:' .. mileage ..', and dis is:'.. poi_type_issued_rules["dis]")
			if mileage <= (tonumber(poi_type_issued_rules["dis"])) then
--				flag = true	
				--记录符合下法距离并且距离最远的一个
				if max_mileage == -1 or max_mileage < mileage then
					index = i--该POI的索引
					max_mileage = mileage
				end
			end
		end
	end

	--没有满足下发条件的项
	if not index then
		return {have_issued = true}
	end

	--local poi_info = jo[index]
	--返回匹配结果(如果有匹配的POI，同时返回该POI的信息)
	only.log("D","leave common_match" .. poi_type)
	return { have_issued = false, poi_info = poi_type_value_table[index] }
end


--
-- 名      称: common_work
-- 功      能: 公共模块
-- 参      数: @poi_type,@match_poi_info,@message_info_list,@poi_type_issued_rules
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--
local function common_work(poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
	only.log("D","in common_work" .. poi_type)
	local lon       = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
        local lat       = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
        local speed     = APP_POOL["OUR_BODY_TABLE"]["speed"] and APP_POOL["OUR_BODY_TABLE"]["speed"][1] or 0
        local direction = APP_POOL["OUR_BODY_TABLE"]["direction"][1]
        local altitude  = APP_POOL["OUR_BODY_TABLE"]["altitude"] and APP_POOL["OUR_BODY_TABLE"]["altitude"][1] or 0
	local direction = APP_POOL["OUR_BODY_TABLE"]["direction"][1]

	local id 	= match_poi_info["positionID"]
	local poi_lon   = match_poi_info["longitude"]
	local poi_lat   = match_poi_info["latitude"]
	local poiID_key	= APP_POOL["OUR_BODY_TABLE"]["accountID"] .. ":POIID"

	--local ok, ret = redis_api.cmd('private_hash', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sismember', poiID_key, id)
	--将该POI TYPE添加到已下发POI类型集合中
	local ok, ret = redis_api.cmd('private_hash', APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', poiID_key, id)
	if not ok then
		error(string.format("work exec failed|poi_type:%s|%s", poi_type, ret))
	end
	
	message_info_list[poi_type] = {}
	if match_poi_info['direction'] then
		message_info_list[poi_type]["receiverDirection"] = string.format('[%s, 45]', match_poi_info["direction"])
	else
		message_info_list[poi_type]["receiverDirection"] = string.format('[%s, 45]', direction or -1)
	end

	message_info_list[poi_type]["senderLongitude"] = lon
	message_info_list[poi_type]["senderLatitude"] = lat
	message_info_list[poi_type]["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
	message_info_list[poi_type]["senderSpeed"] =  string.format('[%s]', speed and math.ceil(speed) or 0)
	if altitude then
		message_info_list[poi_type]["senderAltitude"] =  altitude
	end
	message_info_list[poi_type]["receiverLongitude"] = poi_lon
	message_info_list[poi_type]["receiverLatitude"] = poi_lat
	message_info_list[poi_type]["receiverDistance"] = poi_type_issued_rules["receiver_dis"]
	message_info_list[poi_type]["interval"] = poi_type_issued_rules["interval"]
	message_info_list[poi_type]["level"] =  poi_type_issued_rules["level"] or 35
	message_info_list[poi_type]["context"] = poi_type_issued_rules["context"]
	only.log("D","leave common_work" .. poi_type)

end



--
-- 名      称: POI_SENCE_LIST
-- 功      能: 每种类型POI具体处理函数
-- 参      数: 无
-- 返  回  值: 无
-- 修      改: 新生成函数 yy 2015/6/26
--
POI_SENCE_LIST = {
	--右转红灯
	["1126159"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126159"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126159"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end
			local poi_idx = nil
			local poi_tb = {
				['01']= '易滑路段，天气不好，需更加小心。',
				['02']= '易滑路段，天气不好，需更加小心。',
			}
			poi_idx = "0" .. ( os.time() % 2  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},
	--右转避让行人
	["1126160"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126160"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126160"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '前方路口如遇红灯右转，请降低车速，礼让行人。',
				['02']= '前方路口如遇红灯右转，请降低车速，礼让行人。',
			}
			poi_idx = "0" .. (os.time()%2 + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},

	--动物园
	["1118102"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1118102"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1118102"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '前方路过动物园，请注意游客和行人。',
				['02']= '动物园附近，亲，你有去过吗？',
				['03']= '动物园附近，亲，你有去过吗？',
				['04']= '动物园附近，亲，你有去过吗？',
				['05']= '动物园附近，亲，你有去过吗？',
				['06']= '动物园附近，亲，你有去过吗？',
			}
			poi_idx = "0" .. ( os.time() % 6  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},

	--植物园
	["1118103"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1118103"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1118103"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end


			local poi_idx = nil
			local poi_tb = {
				['01']= '前方路过植物园，请注意游客和行人。',
				['02']= '即将经过植物园，请安全驾驶，压到花花草草也是不对的。',
				['03']= '植物园附近，是不是觉得空气都好一点。',
				['04']= '植物园附近，是不是觉得空气都好一点。',
				['05']= '植物园附近，是不是觉得空气都好一点。',
				['06']= '植物园附近，是不是觉得空气都好一点。',
			}
			poi_idx = "0" .. ( os.time() % 6  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list

		end,
	},

	--驾校
	["1122106"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = 1122106
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = 1122106
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end


			local poi_idx = nil
			local poi_tb = {
				['01']= '驾校附近，请不要尾随教练车。',
				['02']= '传说附近有很多教练车，要小心驾驶噢。',
				['03']= '附近有驾校，马路杀手出没，小心驾驶肯定没错。',
				['04']= '附近有驾校，马路杀手出没，小心驾驶肯定没错。',
				['05']= '附近有驾校，马路杀手出没，小心驾驶肯定没错。',
				['06']= '附近有驾校，马路杀手出没，小心驾驶肯定没错。',
				['07']= '附近有驾校，马路杀手出没，小心驾驶肯定没错。',
				['08']= '附近有驾校，马路杀手出没，小心驾驶肯定没错。',
			}
			poi_idx = "0" .. ( os.time() % 8  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list

		end,
	},

	--中学
	["1122102"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1122102"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1122102"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end


			local poi_idx = nil
			local poi_tb = {
				['01']= '学校路段，降低车速有利于保护儿童。',
				['02']= '学校路段，放学时间车多人多，请注意',
				['03']= '学校路段，放学时间车多人多，请注意',
				['04']= '学校路段，放学时间车多人多，请注意',
				['05']= '学校路段，放学时间车多人多，请注意',
			}
			poi_idx = "0" .. ( os.time() % 5  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},


	--小学
	["1122103"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1122103"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1122103"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end


			local poi_idx = nil
			local poi_tb = {
				['01']= '学校路段，降低车速有利于保护儿童。',
				['02']= '学校路段，放学时间车多人多，请注意',
				['03']= '学校路段，放学时间车多人多，请注意',
				['04']= '学校路段，放学时间车多人多，请注意',
				['05']= '学校路段，放学时间车多人多，请注意',
			}
			poi_idx = "0" .. ( os.time() % 5  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},


	--幼儿园
	["1122104"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1122104"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1122104"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end


			local poi_idx = nil
			local poi_tb = {
				['01']= '学校路段，降低车速有利于保护儿童。',
				['02']= '学校路段，放学时间车多人多，请注意',
				['03']= '学校路段，放学时间车多人多，请注意',
				['04']= '学校路段，放学时间车多人多，请注意',
				['05']= '学校路段，放学时间车多人多，请注意',
			}
			poi_idx = "0" .. ( os.time() % 5  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},
	
	--农贸市场
	["1114101"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1114101"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1114101"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '农贸市场路段，人多，车多。',
				['02']= '前方有农贸市场，亲，要买菜吗？',
				['03']= '前方有农贸市场，亲，要买菜吗？',
				['04']= '前方有农贸市场，亲，要买菜吗？',
			}
			poi_idx = "0" .. ( os.time() % 4  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
	
		end,
	},


	--隧道
	["1123101"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1123101"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1123101"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '即将进入隧道，请开大灯。',
				['02']= '即将进入隧道，请勿随意变道。',
				['03']= '进入隧道，注意光线变化。',
				['04']= '进入隧道，注意光线变化。',
				['05']= '进入隧道，注意光线变化。',
				['06']= '进入隧道，注意光线变化。',
				['07']= '进入隧道，注意光线变化。',
			}
			poi_idx = "0" .. ( os.time() % 7  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx					
			return poi_issued_list
		end,
	},

	--大桥
	["1123107"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1123107"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1123107"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '正在经过桥梁路段，请控制车速',
				['02']= '桥梁路段，多有横风，请勿超速。',
			}
			poi_idx = "0" .. ( os.time() % 2  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},

	--事故易发地段
	["1126111"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126111"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126111"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '该路段事故易发，要打起精神开车。',
				['02']= '注意，事故易发路段，保持合理车距，避免急刹。',
				['03']= '注意，事故易发路段，保持合理车距，避免急刹。',
				['04']= '注意，事故易发路段，保持合理车距，避免急刹。',
				['05']= '注意，事故易发路段，保持合理车距，避免急刹。',
			}
			poi_idx = "0" .. ( os.time() % 5  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},

	--加油站
	["1123105"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1123105"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1123105"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end

			local gas_roadID_poiType_key = APP_POOL["OUR_BODY_TABLE"]["accountID"] .. ':gasStationKey'
			local ok = redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"],
							'set', gas_roadID_poiType_key, poi_type)
			if ok then
				local roadID_poiType_set = APP_POOL["OUR_BODY_TABLE"]["accountID"] .. ':roadIDPOIType'
				redis_api.cmd('driview', APP_POOL["OUR_BODY_TABLE"]["accountID"],'sadd', roadID_poiType_set, roadID)
			end


			local poi_idx = nil
			local poi_tb = {
				['01']= '加油站',
				['02']= '加油站',
				['03']= '加油站',
			}
			poi_idx = "0" .. ( os.time() % 3  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			message_info_list[poi_type]["context"] = '马上会经过一个加油站，请检查油表油量'		
		end,
	},

	--注意落石
	["1126110"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126110"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126110"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '落石',
				['02']= '落石',
			}
			poi_idx = "0" .. ( os.time() % 2  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},

	--村庄
	["1126113"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126113"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126113"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '经过村庄路段，小心岔路来车。',
				['02']= '经过村庄路段，小心岔路来车。',
				['03']= '经过村庄路段，小心岔路来车。',
			}
			poi_idx = "0" .. ( os.time() % 3  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},

	--无人看管的铁路道口
	["1126117"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126117"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126117"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '前方为铁路道口，请谨慎驾驶',
				['02']= '前方为铁路道口，请谨慎驾驶',

			}
			poi_idx = "0" .. ( os.time() % 2  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},

	--道路变窄
	["1126118"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126118"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126118"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '前方道路变窄，请降低车速，小心通行',
				['02']= '前方道路变窄，请降低车速，小心通行',

			}
			poi_idx = "0" .. ( os.time() % 2  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list

		end,
	},

	--向左急弯路
	["1126119"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126119"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126119"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '前方道路变窄，请降低车速，小心通行',
				['02']= '前方道路变窄，请降低车速，小心通行',
				['03']= '前方道路变窄，请降低车速，小心通行',
			}
			poi_idx = "0" .. ( os.time() % 3  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list

		end,
	},

	--向右急弯路
	["1126120"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126120"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126120"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '前方有急转弯，请降低车速，小心通行',
				['02']= '前方有急转弯，请降低车速，小心通行',
				['03']= '前方有急转弯，请降低车速，小心通行',
			}
			poi_idx = "0" .. ( os.time() % 3  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list

		end,
	},

	--连续弯路
	["1126122"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126122"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126122"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '前方有急转弯，请降低车速，小心通行',
				['02']= '前方有急转弯，请降低车速，小心通行',
			}
			poi_idx = "0" .. ( os.time() % 2  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list

		end,
	},

	--左侧合流
	["1126123"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126123"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126123"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '请注意并线车辆，小心驾驶注意避让',
				['02']= '请注意并线车辆，小心驾驶注意避让',
			}
			poi_idx = "0" .. ( os.time() % 2  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list

		end,
	},

	--右侧合流
	["1126124"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126124"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126124"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '请注意并线车辆，小心驾驶注意避让',
				['02']= '请注意并线车辆，小心驾驶注意避让',
			}
			poi_idx = "0" .. ( os.time() % 3  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list

		end,
	},

	--易滑
	["1126112"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126112"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126112"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			local poi_idx = nil
			local poi_tb = {
				['01']= '易滑路段，天气不好，需更加小心。',
				['02']= '易滑路段，天气不好，需更加小心。',
				['03']= '易滑路段，天气不好，需更加小心。',
				['04']= '易滑路段，天气不好，需更加小心。',
			}
			poi_idx = "0" .. ( os.time() % 4  + 1)
			message_info_list[poi_type .. poi_idx] = message_info_list[poi_type]
			message_info_list[poi_type] = nil
			message_info_list[poi_type .. poi_idx]["context"] = poi_tb[tostring(poi_idx)]
			poi_type = poi_type .. poi_idx
			return poi_issued_list
		end,
	},

	--高速跨省收费站
	["1123112"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1123112"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1123112"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			message_info_list[poi_type]["context"] =  match_poi_info["positionID"]
			return poi_issued_list
	
		end,
	},

	--有名大桥
	["1123114"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1123114"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1123114"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			message_info_list[poi_type]["context"] =  match_poi_info["positionID"]
			return poi_issued_list
		end,
	},

	--特殊酒店
	["1117102"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1117102"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1117102"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			message_info_list[poi_type]["context"] =  match_poi_info["positionID"]
			return poi_issued_list
		end,
	},

	--旅游景点
	["1118101"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1118101"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1118101"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			message_info_list[poi_type]["context"] =  match_poi_info["positionID"]
			return poi_issued_list
		end,
	},

	--服务区
	["1123106"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1123106"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1123106"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			message_info_list[poi_type]["context"] =  match_poi_info["positionID"]
			return poi_issued_list
		end,
	},

	--高速出口
	["1126157"] = {
		match = function(poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			local poi_type = "1126157"
			local ok, match_result = pcall(common_match, poi_type, poi_type_issued_table, poi_type_value_table, poi_type_issued_rules)
			if not ok then
				error(string.format("[common_match] failed|[ERROR_MESSAGE]:%s", tostring(ret)))
			end
			return match_result--table类型{true or false,{poi info}}
		end,

		work = function(match_poi_info, roadID, poi_issued_list, message_info_list, poi_type_issued_rules)
			local poi_type = "1126157"
			local ok, ret = pcall(common_work, poi_type, match_poi_info, message_info_list, poi_type_issued_rules)
			if not ok then
				error(string.format("common_work failed|poi_type:%s|%s", poi_type, tostring(ret)))
			end
			if not poi_issued_list then
				poi_issued_list = poi_type
			else
				poi_issued_list = poi_issued_list .. "," .. poi_type
			end

			message_info_list[poi_type]["context"] =  match_poi_info["positionID"]
			return poi_issued_list
		end,
	},

	
	--限速摄像头
	["1123110"] = {
		match = function(camera_issued_table, poiID)
			local have_issued = false
			for _, v in pairs(camera_issued_table or {}) do
				if v == poiID then
					have_issued = true
					break
				end
			end

			return have_issued
		end,

		work = function(poi_info_table, poi_issued_rules, road_type, message_info_list, camera_current_issued_table)
			local poi_type = "1123110"

			local poi_lon   = poi_info_table["longitude"]
			local poi_lat   = poi_info_table["latitude"]
			local id	= poi_info_table["positionID"]
			local key 	= id .. ':POIInfo'

			local direction = APP_POOL["OUR_BODY_TABLE"]["direction"][1]
			local lon       = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
			local lat       = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
			local speed     = APP_POOL["OUR_BODY_TABLE"]["speed"] and APP_POOL["OUR_BODY_TABLE"]["speed"][1] or 0
			local altitude  = APP_POOL["OUR_BODY_TABLE"]["altitude"] and APP_POOL["OUR_BODY_TABLE"]["altitude"][1] or 0

			local ok, rf_info = redis_api.cmd("POIInfo", APP_POOL["OUR_BODY_TABLE"]["accountID"], "hget", key, 'DP')
			if ok and  rf_info then
				rf_info = tonumber(rf_info)
				if rf_info < 30 then
				else
					if poi_issued_rules[poi_type .. rf_info] then
						poi_type = poi_type .. rf_info
					end
				end
			end
			
			if road_type and road_type == 0 then
				poi_issued_rules[poi_type]["dis"] = 1500
                                poi_issued_rules[poi_type]["receiver_dis"] = 1000
			end

			
			message_info_list[poi_type] = {}
                        if poi_info_table['direction'] then
                        	message_info_list[poi_type]["receiverDirection"] = string.format('[%s, 45]', poi_info_table["direction"])
                        else
                                message_info_list[poi_type]["receiverDirection"] = string.format('[%s, 45]', direction or -1)
                        end
			message_info_list[poi_type]["senderLongitude"] = lon
                        message_info_list[poi_type]["senderLatitude"] = lat
			message_info_list[poi_type]["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
                        message_info_list[poi_type]["senderSpeed"] =  string.format('[%s]', speed and math.ceil(speed) or 0)
                        if altitude then
                        	message_info_list[poi_type]["senderAltitude"] =  altitude
                        end
			message_info_list[poi_type]["receiverLongitude"] = poi_lon
                        message_info_list[poi_type]["receiverLatitude"] = poi_lat
			message_info_list[poi_type]["receiverDistance"] = poi_issued_rules[poi_type].receiver_dis
			message_info_list[poi_type]["interval"] = poi_issued_rules[poi_type].interval
			message_info_list[poi_type]["level"] =  poi_issued_rules[poi_type].level or 35
                        if  poi_issued_rules[poi_type].context == "前方经过" then
                        	message_info_list[poi_type]["context"] = "前方经过" .. poi_name     --有问题，poi_name没有赋值
                        else
                        	message_info_list[poi_type]["context"] = poi_issued_rules[poi_type].context
                        end
			camera_current_issued_table[#camera_current_issued_table+1] = id
		end,
	},


	--违章摄像头
	["1123111"] = {
		match = function(camera_issued_table, poiID)
			local have_issued = false
			for _, v in pairs(camera_issued_table or {}) do
				if v == poiID then
					have_issued = true
					break
				end
			end

			return have_issued
		end,

		--poi_issued_list[poi_type] =poi_type_issued_table
		work = function(poi_info_table, poi_issued_rules, road_type, message_info_list, camera_current_issued_table)
			local poi_type = "1123111"

			local poi_lon   = poi_info_table["longitude"]
			local poi_lat   = poi_info_table["latitude"]
			local id	= poi_info_table["positionID"]
			local key 	= id .. ':POIInfo'

			local direction = APP_POOL["OUR_BODY_TABLE"]["direction"][1]
			local lon       = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
			local lat       = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
			local speed     = APP_POOL["OUR_BODY_TABLE"]["speed"] and APP_POOL["OUR_BODY_TABLE"]["speed"][1] or 0
			local altitude  = APP_POOL["OUR_BODY_TABLE"]["altitude"] and APP_POOL["OUR_BODY_TABLE"]["altitude"][1] or 0


                        local poi_idx = "0" .. ( os.time() % 2  + 1)
                        poi_type = poi_type .. poi_idx
			
			message_info_list[poi_type] = {}
                        if poi_info_table['direction'] then
                        	message_info_list[poi_type]["receiverDirection"] = string.format('[%s, 45]', poi_info_table["direction"])
                        else
                                message_info_list[poi_type]["receiverDirection"] = string.format('[%s, 45]', direction or -1)
                        end
			message_info_list[poi_type]["senderLongitude"] = lon
                        message_info_list[poi_type]["senderLatitude"] = lat
			message_info_list[poi_type]["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
                        message_info_list[poi_type]["senderSpeed"] =  string.format('[%s]', speed and math.ceil(speed) or 0)
                        if altitude then
                        	message_info_list[poi_type]["senderAltitude"] =  altitude
                        end
			message_info_list[poi_type]["receiverLongitude"] = poi_lon
                        message_info_list[poi_type]["receiverLatitude"] = poi_lat
			message_info_list[poi_type]["receiverDistance"] = poi_issued_rules[poi_type].receiver_dis
			message_info_list[poi_type]["interval"] = poi_issued_rules[poi_type].interval
			message_info_list[poi_type]["level"] =  poi_issued_rules[poi_type].level or 35
                        if  poi_issued_rules[poi_type].context == "前方经过" then
                        	message_info_list[poi_type]["context"] = "前方经过" .. poi_name     --有问题，poi_name没有赋值
                        else
                        	message_info_list[poi_type]["context"] = poi_issued_rules[poi_type].context
                        end
			camera_current_issued_table[#camera_current_issued_table+1] = id
		end,
	},
}
