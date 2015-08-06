-- author       : malei
-- date         : 2014-9-23
-- Feature No   : 2163

local only			= require ('only')
local redis_api			= require ('redis_pool_api')
local http_api			= require ('http_short_api')
local mysql_api			= require ('mysql_pool_api')
local fun_point_match_road	= require ('spx_point_match_road')
local utils			= require ('utils')
local scan			= require ('scan')
local json			= require ('cjson')
local lua_decoder		= require ('libluadecoder')
local link			= require ("link")
local raidus_find_poi_func	= require ('raidus_find_poi_func')
local socket			= require ('socket')
local APP_POOL			= require('pool')

local TEN_MINUTES		= 600
local FIVE_HOURS		= 3600*5
local TWENTY_HOURS		= 3600*6 -- 3600*20
local FORTY_EIGHT_HOURS		= 3600*48
local TEN_MINUTES	        = 600
local TWO_HOURS	                = 3600*2

local G_OLD_IDX_TIME		= 1
local G_OLD_IDX_LON		= 7
local G_OLD_IDX_LAT		= 8
local G_OLD_IDX_DIR		= 10
local G_OLD_IDX_SPEED		= 11
local G_OLD_IDX_TOKENCODE	= 12

local G_NEW_IDX_TIME		= 1
local G_NEW_IDX_LON		= 2
local G_NEW_IDX_LAT		= 3
local G_NEW_IDX_DIR		= 4
local G_NEW_IDX_SPEED		= 5

local EARTH_RADIUS		= 6378.137  --  地球半径
local MAX_AVERAGE_SPEED		= 130       --  判断数据异常
local MIN_GAP_SPEED		= 40        --  前后秒速度差，判断数据异常
local PACKAGE_TIME		= 11        --  11s两个包数据
local RADIUS_POI		= 200       --  poi搜索半径


local NEXT_ROAD_AFFIRM_MAX_STEP = 5
local LINE_POINT_RANGE		= 0.002     --  公司早期weme数据统计结果
local RANG_POINT_PROPORTION	= 0.8       --  公司早期weme数据统计结果
local ONE_RANGE_TIME		= 600       --  公司早期weme数据统计结果


local lua_socket_point_min	= 100
local lua_socket_point_max	= 0
local lua_socket_point_sum	= 0


local _g = {
	select_tokencode_from_config = "select tokenCode,createTime from userPowerOnInfo where accountID = '%s' and createTime > %s; ",
}

module('stsv_gopath', package.seeall)


local function memory_analyse( tag )
	local msize = collectgarbage("count")
	local ldata = string.format("%s : memory use \t[%d]KB \t[%d]M", tag or "", msize, msize/1024)
	print(ldata)
	only.log('I', ldata)
end

local function get_nearby_poi(store, i)
	local tb = {
		longitude = store[i][G_NEW_IDX_LON],
		latitude = store[i][G_NEW_IDX_LAT],
		number = 1,
		radius = RADIUS_POI,
	}
	local res = raidus_find_poi_func.handle(tb)
	--[[
	]]--
	if res then
		return res['name']
	end

	return ""
end
--[[
local function get_data_from_redis(store, imei, st_frame, ed_frame)
	local set_keys = {}
	for frame = st_frame, ed_frame, TEN_MINUTES do
		local time_key = os.date('%Y%m%d%H%M', frame)
		table.insert(set_keys, string.format("GPS:%s:%s", imei, string.sub(time_key, 1, -2)))
	end
	local ok, data = redis_api.cmd('newStatus', imei, 'SUNION', unpack(set_keys))
	if not ok or not data then
		only.log("E", "redis sunion failed!")
		return false
	end
	if #data == 0 then
		return true
	end
	for k,v in ipairs(data) do
		v = string.gsub(v, "||", "|a|")
		data[k] = utils.str_split(v, "|")
	end
	table.sort(data, function(a,b)
		return a[G_NEW_IDX_TIME] < b[G_NEW_IDX_TIME]
	end)
	for _,v in ipairs(data) do
		table.insert(store, v)
	end
	return true
end

local function get_data_from_tsldb(store, imei, st_frame, ed_frame)
	local st_key = os.date('%Y%m%d%H%M', st_frame)
	local ed_key = os.date('%Y%m%d%H%M', ed_frame)
	local field = string.format("GPS:%s:", imei)

	local ok, data = redis_api.cmd('tsdb', imei, 'LRANGE', field,
	string.sub(st_key, 1, -2), string.sub(ed_key, 1, -2))
	if not ok or not data then
		only.log("E", "tsdb lrange failed!")
		return false
	end
	if #data == 0 then
		return true
	end
	for k,_ in ipairs (data) do
		table.remove(data, k)
	end
	local info = lua_decoder.decode(#data, data)
	if not info then
		return false
	end
	for _,v in ipairs(info) do
		table.insert(store, v)
	end
	return true
end

local function dispatch_to_split_data(store, imei, st_time, ed_time, get_data_fcb)
	local st_frame = 0
	local ed_frame = st_time - TEN_MINUTES
	repeat
		st_frame = ed_frame + TEN_MINUTES
		ed_frame = st_frame + FIVE_HOURS
		if ed_frame > ed_time then
			ed_frame = ed_time
		end
		local ok = get_data_fcb(store, imei, st_frame, ed_frame)
		if not ok then
			return false
		end
	until ed_frame == ed_time
	return true
end

local function dispatch_to_whole_data(store, imei, st_time, ed_time)
	local redis_have = false
	local tsldb_have = false
	local dv_time = 0
	local now = os.time()
	if (now - st_time) > TWENTY_HOURS then
		if (now - ed_time) > TWENTY_HOURS then
			tsldb_have = true
			redis_have = false
			dv_time = ed_time
		else
			tsldb_have = true
			redis_have = true
			dv_time = now - TWENTY_HOURS
		end
	else
		tsldb_have = false
		redis_have = true
		dv_time = st_time
	end
	if tsldb_have then
		local ok = dispatch_to_split_data(store, imei, st_time, dv_time, get_data_from_tsldb)
		if not ok then
			return false
		end
	end
	if redis_have then
		local ok = dispatch_to_split_data(store, imei, dv_time, ed_time, get_data_from_redis)
		if not ok then
			return false
		end
	end
	return true
end
]]--

-->通过tsearchapi接口获取数据
local function get_data_from_tserchapi(store, imei, st_frame, ed_frame)
	local body_info = {imei = imei, startTime = st_frame, endTime = ed_frame}

	local serv = link["OWN_DIED"]["http"]["tsearchapi/v2/getgps"]
	local body = utils.gen_url(body_info)
	local data = utils.compose_http_json_request(serv, "tsearchapi/v2/getgps", nil, body)
	only.log('D',"data : " .. data)

	local ok, ret = supex.http(serv['host'], serv['port'], data, #data)
	if not ok or not ret then return nil end
	only.log('D', ret)
	-->获取RESULT后的数据	
	local data = utils.parse_api_result(ret, "getgps")
	if not data then
                return nil
        end
        if #data == 0 then return true end
        for k,_ in ipairs (data) do
		table.remove(data, k)
	end
	local info = lua_decoder.decode(#data, data)
	if not info then
		return false
	end

	for _,v in ipairs(info) do
		table.insert(store, v)
	end
	return true
end

-->每次从tsearchapi中取2个小时数据，直到取完
local function get_whole_data(store,imei,st_time,ed_time)
	local st_frame = 0
	local ed_frame = st_time - TEN_MINUTES

	repeat
		st_frame = ed_frame + TEN_MINUTES
		ed_frame = st_frame + TWO_HOURS
		if ed_frame > ed_time then
			ed_frame = ed_time
		end
		local ok = get_data_from_tserchapi(store, imei, st_frame, ed_frame)
		if not ok then
			return false
		end
	until ed_frame == ed_time
	return true
end





local function gps_data_get_road_id(store, i)
	local lon = store[i][G_NEW_IDX_LON]
	local lat = store[i][G_NEW_IDX_LAT]
	local dir = store[i][G_NEW_IDX_DIR]

	local t1 = socket.gettime()
	local ok,result = fun_point_match_road.entry(dir, lon, lat, _g['accountID'])
	local t2 = socket.gettime()

	local t3 = t2 - t1
	--only.log('D', string.format("[luaSocketPoint][elapse = %s]", t3))
	if lua_socket_point_min > t3 then
		lua_socket_point_min = t3
	end
	if lua_socket_point_max < t3 then
		lua_socket_point_max = t3
	end

	lua_socket_point_sum = lua_socket_point_sum + t3

	if ok and result then
		return result['roadID']
	end
	return nil
end

local function gps_data_get_road_info(roadID)
	if roadID == 0 then
		return nil
	end
	local key = roadID .. ":roadInfo"
	local ok, kv_tab = redis_api.cmd('mapRoadInfo', "", 'hgetall', key)
	--print(string.format("[kv_tab:%s]", scan.dump(kv_tab)))
	if  ok and kv_tab then
		return kv_tab
	else
		only.log('D', string.format("[hgetall road info error]"))
	end
	return nil
end
local function direction_sub(dir1, dir2)
	local angle = math.abs(dir1 - dir2)
	return (angle <= 180) and angle or (360 - angle)
end

local function delete_stop_region_point (store, i, j)
	local cnt = j - i + 1
	local pre = -1
	local crt = -1
	local stop = false

	for k=1, cnt do
		pre = crt
		crt = tonumber(store[i][G_NEW_IDX_DIR])
		if crt == -1 then
			if not stop then
				stop = true
				i = i + 1
			else
				table.remove(store, i)
			end
		else
			stop = false
			if pre == -1 then
				i = i + 1
			else
				if 30 > direction_sub(pre, crt) then
					i = i + 1
				else
					table.remove(store, i)
				end
			end
		end
	end
	return i
end

local function gps_data_filter_stop_region(store, idx)
	local lat_min   = store[idx][G_NEW_IDX_LAT] - LINE_POINT_RANGE
	local lat_max   = store[idx][G_NEW_IDX_LAT] + LINE_POINT_RANGE
	local lon_min   = store[idx][G_NEW_IDX_LON] - LINE_POINT_RANGE
	local lon_max   = store[idx][G_NEW_IDX_LON] + LINE_POINT_RANGE
	local start_time = store[idx][G_NEW_IDX_TIME]

	local i = idx
	local j = idx
	local cnt = 0
	local ptr = store[idx]
	repeat
		j = i
		if ptr[G_NEW_IDX_LAT] >= lat_min and ptr[G_NEW_IDX_LAT] <= lat_max
			and ptr[G_NEW_IDX_LON] >= lon_min and ptr[G_NEW_IDX_LON] <= lon_max then
			cnt = cnt + 1
		end
		i = j + 1
		ptr = store[i]
	until (not ptr) or (ptr[G_NEW_IDX_TIME] - start_time > ONE_RANGE_TIME)

	if j > idx and cnt/(i - idx) > RANG_POINT_PROPORTION then
		return delete_stop_region_point (store, idx, j)
	else
		return i
	end
end

local function gps_data_filter_direction(store)
	local stop = false
	local idx = 1
	local dir = -1
	for i=1,#store do
		dir = store[idx][G_NEW_IDX_DIR]
		if dir == -1 then
			if stop == false then
				stop = true
				idx = idx + 1
			else
				table.remove(store, idx)
			end
		else
			stop = false
			idx = idx + 1
		end
	end
end



-- 过滤原则
-- 001. tokenCode过滤，错误数据过滤,非需要字段过滤
-- 002. 设备静止过滤，dir！=-1且前后点dir差小于30的非飘数据保留
-- 003. dir = -1的过滤，第一个-1点参加gps定位，
--      其他不参加gps定位，但是全部参与里程计算

local function gps_data_filter(old_store, new_store)
	-->> filter tokenCode
	memory_analyse( "[old_store]" )
	only.log('D', string.format("[#old_store = %d]", #old_store))
	for i,v in ipairs(old_store or {}) do
		if v[G_OLD_IDX_TOKENCODE] == _g['tokenCode'] and #v == 12 then
			table.insert(new_store, {
				[G_NEW_IDX_TIME] = tonumber(v[G_OLD_IDX_TIME]),
				[G_NEW_IDX_LON] = tonumber(v[G_OLD_IDX_LON])/10000000,
				[G_NEW_IDX_LAT] = tonumber(v[G_OLD_IDX_LAT])/10000000,
				[G_NEW_IDX_DIR] = tonumber(v[G_OLD_IDX_DIR]),
				[G_NEW_IDX_SPEED] = tonumber(v[G_OLD_IDX_SPEED])
			})
		end
	end
	memory_analyse( "[old_store and new_store]" )
	old_store = nil
	collectgarbage("collect")
	memory_analyse( "[new_store]" )
	-->> filter stop
	local i = 1
	only.log('D', string.format("[#new_store = %d]", #new_store))
	while new_store[i] do
		i = gps_data_filter_stop_region(new_store, i)
	end
	if #new_store > 1 then
		gps_data_filter_direction(new_store)
	end
	only.log('D', string.format("[#new_store = %d]", #new_store))
	memory_analyse( "[filter new_store]" )
	return
end

-- 获取下一个roadID的起始点
local function gps_data_get_next_road_starting_point(point_match_road_set, start)
	local fst_roadID = point_match_road_set[start]
	local step = 0
	local tmp_roadID = nil
	local point = start + 1

	for i = start + 1, #point_match_road_set do
		local cmp_roadID = point_match_road_set[i]
		if step == 0 then
			if cmp_roadID ~= fst_roadID then
				tmp_roadID = cmp_roadID
				step = 1
				point = i
			end
		else
			if cmp_roadID == tmp_roadID then
				step = step + 1
			else
				if cmp_roadID ~= fst_roadID then
					tmp_roadID = cmp_roadID
					step = 1
					point = i
				else
					step = 0
				end
			end
			if step >= NEXT_ROAD_AFFIRM_MAX_STEP then
				--print(string.format("[return][point = %s]", point))
				return point
			end
		end
	end
	return #point_match_road_set
end

local function gps_data_get_point_to_point_dist(store, m, n )
	local F_lon, F_lat =    store[m][G_NEW_IDX_LON], store[m][G_NEW_IDX_LAT]
	local T_lon, T_lat =    store[n][G_NEW_IDX_LON], store[n][G_NEW_IDX_LAT]
	if F_lon == T_lon and  F_lat == T_lat then
		return 0
	end
	local dist = EARTH_RADIUS*math.acos(math.sin(F_lat/57.2958)*
	math.sin(T_lat/57.2958)+math.cos(F_lat/57.2958)*
	math.cos(T_lat/57.2958)*math.cos((F_lon-T_lon)/57.2958))
	*1000
	if not dist  then
		dist = 0
	end
	if dist / (store[n][G_NEW_IDX_TIME]- store[m][G_NEW_IDX_TIME]) >= MAX_AVERAGE_SPEED then
		dist = 0
	end
	return dist
end

local function gps_data_get_a_road_mileage(store, from, over)
	local mileage           = 0
	local actual_mileage    = 0

	local nxt_speed    = store[from][G_NEW_IDX_SPEED]
	local nxt_time     = store[from][G_NEW_IDX_TIME]
	for i = from + 1, over do
		local prv_speed   = nxt_speed
		local prv_time    = nxt_time
		nxt_speed   = store[i][G_NEW_IDX_SPEED]
		nxt_time    = store[i][G_NEW_IDX_TIME]

		local speed_gap =  math.abs(nxt_speed - prv_speed)
		local time_gap  =  math.abs(nxt_time - prv_time)
		if (time_gap == 1 and  speed_gap < MIN_GAP_SPEED) or
			(time_gap > 1 and time_gap <= PACKAGE_TIME) then
			local mid = (((nxt_speed + prv_speed)/2) * time_gap) / 3.6
			mileage = mileage + mid
			actual_mileage = actual_mileage + mid
		elseif time_gap > 11 and time_gap < ONE_RANGE_TIME then
			actual_mileage = actual_mileage + gps_data_get_point_to_point_dist(store, i - 1, i )
		end
	end

	return math.floor(mileage), math.floor(actual_mileage)
end


local function gps_data_calculate_mileage(point_match_road_set, store, road_set)
	local prv = 1
	local nxt = 1
	local idx = 1

	local max = #store
	while prv < max do
		local nxt = gps_data_get_next_road_starting_point(point_match_road_set, prv)
		
		--> compute speed
		local min_speed = 1000
		local max_speed = 0
		local sum_speed = 0
		local avg_speed = 0
		for i = prv, nxt - 1 do
			local speed = store[i][G_NEW_IDX_SPEED]
			if speed < min_speed then
				min_speed = speed
			end
			if speed > max_speed then
				max_speed = speed
			end
			sum_speed = sum_speed + speed
		end
		local cnt = nxt - prv
		avg_speed = sum_speed/((cnt > 0) and cnt or 1)

		--> assemble info
		local mileage, actual_mileage = gps_data_get_a_road_mileage(store, prv, nxt)
		road_set[idx] = {
			['mileage']		= mileage,
			['actualMileage']	= actual_mileage,
			['roadID']		= point_match_road_set[prv],
			['startPOIName']	= get_nearby_poi(store, prv) or '',
			['startLongitude']	= store[prv][G_NEW_IDX_LON] * 10000000,
			['startLatitude']	= store[prv][G_NEW_IDX_LAT] * 10000000,
			['startTime']		= store[prv][G_NEW_IDX_TIME],
			['endPOIName']		= get_nearby_poi(store, nxt) or '',
			['endLongitude']	= store[nxt][G_NEW_IDX_LON] * 10000000,
			['endLatitude']		= store[nxt][G_NEW_IDX_LAT] * 10000000,
			['endTime']		= store[nxt][G_NEW_IDX_TIME],
			['minimumSpeed']	= min_speed or 0,
			['maximumSpeed']	= max_speed or 0,
			['averageSpeed']	= avg_speed or 0,
			['roadIDIndex']		= idx,
			['accountID']		= (_g['accountID'] ~= _g['imei']) and _g['accountID'] or '',
			['imei']		= _g['imei'],
			['tokenCode']		= _g['tokenCode']
		}

		prv = nxt
		idx = idx + 1
	end
end


local function calculate_path(old_store)

	local new_store = {}
	local point_match_road_set = {}
	local road_set = {}

	setmetatable(new_store, { __mode = "k" })
	setmetatable(point_match_road_set, { __mode = "k" })
	setmetatable(road_set, { __mode = "k" })

	local t1 = os.time()
	gps_data_filter(old_store, new_store)
	local t2 = os.time()
	only.log('D', string.format("[filter  time(elapse) = %d]", t2 - t1))

	local max_point_num = #new_store
	if max_point_num > 1 then
		-->> point match road
		memory_analyse( "<calculate_path step 1>" )
		for k,v in ipairs(new_store or {}) do
			point_match_road_set[k] = gps_data_get_road_id(new_store, k) or 0
		end
		local t3 = os.time()
		only.log('D', string.format("[pointMatchRD(elapse) = %d]", t3 - t2))
		only.log('D', string.format("[Each MatchRD(elapse) = %s]", (t3-t2)*1.0/max_point_num))

		only.log('D', string.format("[lua_socket_point_sum(elapse) = %s]", lua_socket_point_sum))
		only.log('D', string.format("[lua_socket_point_max(elapse) = %s]", lua_socket_point_max))
		only.log('D', string.format("[lua_socket_point_min(elapse) = %s]", lua_socket_point_min))
		only.log('D', string.format("[lua_socket_point_avg(elapse) = %s]", lua_socket_point_sum/max_point_num))
		memory_analyse( "<calculate_path step 2>" )

		-->> compute mileage
		gps_data_calculate_mileage(point_match_road_set, new_store, road_set)
		point_match_road_set = nil
		new_store = nil
		collectgarbage("collect")
		local t4 = os.time()
		only.log('D', string.format("[calc    time(elapse) = %d]", t4 - t3))

		memory_analyse( "<calculate_path step 3>" )
		for i,v in pairs(road_set or {}) do

			local roadID = v['roadID']
			local kv_tab = gps_data_get_road_info(roadID)
			if kv_tab then

				road_set[i]['roadName']        =   kv_tab['RN'] or "newRoad"
				road_set[i]['provinceCode']    =   kv_tab['PC']
				road_set[i]['countyCode']      =   kv_tab['CC']
				road_set[i]['limitSpeed']      =   kv_tab['SR']
				road_set[i]['provinceName']    =   kv_tab['PN']
				road_set[i]['roadType']        =   kv_tab['RT']
				road_set[i]['cityName']        =   kv_tab['CA']
				road_set[i]['roadRootID']      =   '0'
				road_set[i]['countyName']      =   kv_tab['CN']
				road_set[i]['cityCode']        =   kv_tab['CD']
			else
				road_set[i]['roadName']        =   "newRoad"
			end
		end
		local t5 = os.time()
		only.log('D', string.format("[get INF time(elapse) = %d]", t5 - t4))
		memory_analyse( "<calculate_path step 4>" )
	else
		--- for no point for calculate path
		road_set = {}
		local a= {}
		a['tokenCode'] = _g['tokenCode']
		if _g['imei'] == _g['accountID'] then
			a['accountID'] = ''
		else
			a['accountID'] = _g['accountID']
		end
		a['imei'] = _g['imei']
		table.insert(road_set, a)
	end

	local ok, res = pcall(json.encode, road_set)
	if not ok or not res then
		only.log('E', "decode error")
	end
	road_set = nil
	collectgarbage("collect")

	res =string.format('{"ERRORCODE":"0","RESULT":%s}',res)

	local tab = {
		jsonRoad = res,
		appKey = "3619608887" ,
	}
	--local secret_v = "AEFA154AA9890EC4A9E0E49A3E9FE08C859D5844"
	--tab['sign'] = utils.gen_sign(tab, secret_v)
	local body_data =  utils.table_to_kv(tab)
	local app_server = link["OWN_DIED"]["http"]["DataCore/autoGraph/addTravelRoadInfo"]
	local data = utils.post_data("DataCore/autoGraph/addTravelRoadInfoTest", app_server, body_data)
	only.log('D', string.format("data:%s", data))
	local http_time = os.time()
	only.log('I', string.format("[tokenCode = %s],[callback time = %d]", _g['tokenCode'], http_time))
	supex.http(app_server['host'], app_server['port'], data, #data)
	--[[
	--local data = utils.post_data("DataCore/autoGraph/addTravelRoadInfo", app_server, body_data)
	--print(string.format("data:%s", data))
	http_api.http(app_server, data, false)
	--write_log(string.format("[data:%s]", data))
	--]]

end


function handle()
	--only.log('D', scan.dump(...))
	local args = APP_POOL["OUR_BODY_TABLE"]

	local accountID = args['accountID']
	local imei      = args['imei']
	local tokenCode = args['tokenCode']
	local startTime = args['startTime']
	local endTime   = args['endTime']


	-->> 35个点 1s
	--[[
	imei		= "800981013883924"
	startTime	= "1404367200"
	endTime		= "1404370800"
	tokenCode	= "lN6GQBBCo5"
	]]--

	-->> 11 hours 10428,10427 178s
	--[[
	imei		= "392227436589951"
	startTime	= "1413406403"
	endTime		= "1413447727"
	tokenCode	= "2iifiYhlKL"
	]]--

	-->> more 24 hours 922s
	--[[
	imei		= "860122717752589"
	startTime	= "1413784067"
	endTime		= "1414025017"
	tokenCode	= "HnHpllpimZ"
	]]--

	-->> 2014-10-23 48小时 01
	--[[
	imei		= "381897734768102"
	startTime	= "1413851021"
	endTime		= "1414023791"
	tokenCode	= "IqDNr1NpKT"
	]]--

	-->> 2014-10-23 48小时 02
	--[[
	imei		= "848573126973603"
	startTime	= "1413840442"
	endTime		= "1414013203"
	tokenCode	= "zmfAnpZHwl"
	]]--

	memory_analyse( "APPLY CALL COME" )
	if imei and tokenCode and startTime and endTime then

		local t1 = os.time()
		local ok, accountID = redis_api.cmd('private', imei, 'get', string.format('%s:accountID', imei))
		if not ok or not accountID then
			only.log('W', "get accountID failed!")
			accountID = imei
		end

		local origin = tonumber(startTime)
		local finish = tonumber(endTime)
		if (finish <= origin) or (finish > (origin + TWO_HOURS * 24)) then
			only.log('S', string.format("ERROR todo task imei %s tokenCode %s from %s to %s", imei, tokenCode, origin, finish))
			return false
		end

		local st_time = origin - (origin % TEN_MINUTES)
		local ed_time = finish - (finish % TEN_MINUTES)

		_g['imei'] = imei
		_g['accountID'] = accountID
		_g['tokenCode'] = tokenCode

		local store = {}
		setmetatable(store, { __mode = "k" })
		--local ok = dispatch_to_whole_data(store, imei, st_time, ed_time)
                local ok = get_whole_data(store, imei, st_time, ed_time)
		if not ok then
			return false
		end
		local t2 = os.time()
		only.log('D', string.format("[#store = %d]", #store))
		--> 过滤掉非时间段内的数据
		if #store > 1 then
			while #store > 0 and tonumber(store[1][G_OLD_IDX_TIME]) < origin do
				table.remove(store, 1)
			end
			while #store > 0 and tonumber(store[ #store ][G_OLD_IDX_TIME]) > finish do
				table.remove(store, #store)
			end
		end
		memory_analyse( "APPLY CALL GAIN" )

		calculate_path(store)
		local t3 = os.time()
		--only.log('D', string.format("[get data time(elapse) = %d],[do data time(elapse) = %d]", t2 - t1, t3 - t2))
		only.log('I', string.format("[get data time(elapse) = %d],[do data time(elapse) = %d], [request to the end of time = %d],[tokenCode = %s],[shutdown time = %s],[start request time =%s]", t2 - t1, t3 - t2, t3-t1,tokenCode, os.date('%Y-%m-%d %H:%M:%S', endTime), os.date('%Y-%m-%d %H:%M:%S', t1)))
		memory_analyse( "APPLY CALL PEAK" )

		lua_socket_point_min = 100
		lua_socket_point_max = 0
		lua_socket_point_sum = 0

	end
	memory_analyse( "APPLY CALL DONE" )
	collectgarbage("collect")
	memory_analyse( "APPLY CALL OVER" )
end
