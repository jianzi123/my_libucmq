local pool = require('pool')
local APP_POOL= require('pool')
local only = require('only')
local supex = require('supex')
local utils = require('utils')
local link = require('link')
local cjson = require('cjson')
local weibo_api = require('weibo')
--local redis_api = require('redis_short_api')
local redis_api = require('redis_pool_api')
local OFFSITE_CFG = require('cfg_offsite_remind')
local weibo_statistics = require('weibo_statistics')

module('p2p_offsite_remind', package.seeall)

--[[
221.228.231.88:3005/getVoiceByTagName
[1] startPage，keywords，pageCount
[2] startTime，keywords, count,

"id": 1381,
"content": "上海的小吃，有蒸、煮、炸，品种很多，最为消费者喜爱的就是汤包、百叶、油面精。这是人们最青睐的三主件",
"fileURL": "http://g3.tweet.daoke.me/group3/M01/0B/FC/rBALa1OZMqqAe6olAAAefgQjaus008.amr",
"fileID": "9798a4f3386e459fb65b39a4a63da44e",
"tag": "|上海|异地特色菜|小吃|土特产",
"createTime": 1402548955
]]--

local function get_stone_message( method, before, mark )
	local serv
	local method_func = {
		alone = function () 
			serv = link["OWN_DIED"]["http"]["getVoiceByTagName"]
			return utils.compose_http_kvps_request(serv, "getVoiceByTagName", nil,
			{
				startTime = before,
				--keywords = mark,
				tagName = mark,
				count = 1,
			},
			"POST")
		end,
		group = function () 
			serv = link["OWN_DIED"]["http"]["getVoiceByTagID"]
			return utils.compose_http_kvps_request(serv, "getVoiceByTagID", nil,
			{
				startTime = before,
				tagID = mark,
				count = 1,
			},
			"POST")
		end
	}
	local req = method_func[ method ]( )
	only.log('D', req)

	local ok, resp = supex.http(serv["host"], serv["port"], req, #req)
	if not ok or not resp then return false,nil end
	only.log('D', resp)

	local jo = utils.parse_api_result( resp, "getVoiceByTagName")
	if not jo then
		return false,nil
	end
	if #jo == 0 then
		return true,nil
	end
	return true,jo[1]
end


local function do_main( accountID, method, city, index, label_name )
	local offsite_key = string.format("%s:%s:%s:%d:offsiteReadTimestamp", accountID, method, city, index)
	local offsite_set = string.format("%s:%s:%s:%d:offsiteReadFileidSet", accountID, method, city, index)
	local offsite_all = string.format("%s:offsiteAllKeysSet", accountID)
	--> get history
	local ok, history = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "get", offsite_key)
	if not ok then return end
	local before = history or os.time()
	--> get fileurl
	only.log("D", string.format("[--label_name:%s]",label_name))
	local ok, info = get_stone_message( method, before, label_name )
	if not ok then return end
	if not info then
		-->| NO find
		if history then
			redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", offsite_key)
		end
	else
		-->| HAS find
		local ok, exist = redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sismember", offsite_set, info["fileID"])
		if not ok then return end
		if exist then
			-->| repeat
			if history then
				redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "del", offsite_key)
			end
		else
			-->| is new
			redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "set", offsite_key, info["createTime"])
			redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sadd", offsite_set, info["fileID"])
			redis_api.cmd("driview",APP_POOL["OUR_BODY_TABLE"]["accountID"], "sadd", offsite_all, offsite_key, offsite_set)
			only.log("D", string.format("[--info[content]:%s]",info["content"]))
			-->| send weibo
			local wb = {
				multimediaURL = info["fileURL"],
				receiverAccountID = accountID,
				interval = 40,
				level = 30,
				content = info["content"],
				senderType = 2,
			}
			local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
			local ok, ret = weibo_api.send_weibo( server, "personal", wb, "1415846010", "A6894BB85049159DCFCFD14277EF464F50C416BF" )
			if ok then
                                weibo_statistics.Statistics(ret['bizid'],"1415846010")
                        --[[
				local bizid = ret['bizid']
				local time = os.time()
				local travelID  = nil
				local appKey = "1415846010"
				local ok, imei =  redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:IMEI", accountID))
				if ok and imei then
					ok, travelID = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:travelID", imei))
				end
				local ok_date, cur_date = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:speedDistribution", accountID))
				if not ok_date or not cur_date then
					cur_date = os.date("%Y%m")
					redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'set', string.format("%s:speedDistribution", accountID), cur_date)
				end 
				if ok and imei and accountID and travelID and cur_date then
					local datacore_statistics_var_key = accountID .. ":" .. travelID .. ":" .. appKey ..":" .. cur_date
					local datacore_statistics_var_value = bizid .. ":" .. time
					redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'sadd', datacore_statistics_var_key, datacore_statistics_var_value)
					redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'expire', datacore_statistics_var_key, 48*3600)

				end
                        ]]--
			end
		end
	end
end

function handle()
	local gps_data = pool["OUR_BODY_TABLE"]
	local accountID = gps_data["accountID"]
	local lon = gps_data["longitude"][1]
	local lat = gps_data["latitude"][1]

	local grid = string.format('%d&%d', tonumber(lon) * 100, tonumber(lat) * 100)
	only.log("D", "redis grid key:" .. grid)
	local ok, code_jo = redis_api.cmd('mapGridOnePercent',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', grid)
	if not ok then
		only.log("W", "can't get code from redis!")
		return
	end
	only.log("D", "json data:" .. code_jo)
	local ok, code_tab = pcall(cjson.decode, code_jo)
	if not ok then
		only.log("E", "can't decode code! " .. code_tab)
		return
	end
	--local countyName = code_tab['countyName']
	--local cityName = code_tab['cityName']
	--local provinceName = code_tab['provinceName']
	local city = tostring(code_tab['cityCode'])
	only.log("D",string.format( "[--cityCode:%s]",city))
	if false then
		for i,v in ipairs( OFFSITE_CFG["OFFSITE_LABEL_LIST_ALONE"][ city ] or {} ) do
			do_main( accountID, "alone", city, i, v )
		end
		for i,v in ipairs( OFFSITE_CFG["OFFSITE_LABEL_LIST_GROUP"][ city ] or {} ) do
			do_main( accountID, "group", city, i, v )
		end
	else
		--> one time only can send one weibo.
		--> local city and local day only can send n group.
		--> when at home city and poweron or poweroff happen, clean all keys.
		local MAX_READ_GROUP = 2
		local group_id_key = string.format("%s:%s:%s:offsiteReadGroupID", accountID, "alone", city)
		local index_id_key = string.format("%s:%s:%s:offsiteReadIndexID", accountID, "alone", city)
		local ok, group_id, index_id
		local first = false
		ok, group_id = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', group_id_key)
		if not ok then return end
		if not group_id then
			first = true
		end
		group_id = tonumber(group_id) or 1
		if group_id <= MAX_READ_GROUP then
			if not first then
				ok, index_id = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', index_id_key)
				if not ok then return end
			end
			index_id = tonumber(index_id) or 1
			local fetch_list = OFFSITE_CFG["OFFSITE_LABEL_LIST_ALONE"][ city ]
			local all = #fetch_list
			if index_id <= all then
				do_main( accountID, "alone", city, index_id, fetch_list[ index_id ] )
			end
			--> set next
			local next_id = index_id + 1
			if next_id > all then
				next_id = 1
				group_id = group_id + 1
			end
			if first or next_id <= index_id then
				local over = 86400 - ((os.time() + 28800)%(86400))
				redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', group_id_key, group_id)
				redis_api.cmd("private",APP_POOL["OUR_BODY_TABLE"]["accountID"], "expire", group_id_key, over)
			end
			redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', index_id_key, next_id)
		end

	end
	return
end
