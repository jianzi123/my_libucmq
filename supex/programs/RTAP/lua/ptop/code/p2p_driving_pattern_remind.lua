local pool = require('pool')
local APP_POOL= require('pool')
local only = require('only')
local supex = require('supex')
local utils = require('utils')
local scan = require('scan')
local link = require('link')
local map = require('map')
local sha = require('sha1')
local cjson = require('cjson')
local weibo_api = require('weibo')
--local redis_api = require('redis_short_api')
local redis_api = require('redis_pool_api')

module('p2p_driving_pattern_remind', package.seeall)

local dianping_srv = link.OWN_DIED["http"]["dianping_server"]

local app_info = {
	appKey = "699771737",
	secret = "9296ED2B9D139D4730484AE4E01C6F17AE2685EA",
}

local dianping_info = {
	appKey = "1740038396",
	secret = "8ec52fedbcaf454e82d117eb8dd8c8a0",
	get_coupon_cities = "/v1/metadata/get_cities_with_coupons",
	get_coupon_list = "/v1/coupon/find_coupons",
	get_business_categories = "/v1/metadata/get_categories_with_businesses",
	get_business_list = "/v1/business/find_businesses",
	request = 'GET %s?%s HTTP/1.0\r\nUser-Agent: curl/7.32.0\r\n'
	.. 'Host: api.dianping.com\r\nAccept: */*\r\n\r\n' ,
}

local function get_nickname(accountID)
	local ok, nickname = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', 
	accountID .. ':nickname')
	if not ok or not nickname then
		nickname = '道客' 
	end
	return nickname
end

local function http_request(server, data)
	--only.log('D', '[http_request]')
	local ok, resp = supex.http(server.host, server.port, data, #data)
	if not ok then
		only.log('D',string.format("http_request is error"))
		resp = nil
	end
	return resp
end

local function get_dianping_sign(wb)
	local sign_str = dianping_info['appKey']
	local wb_key = {}
	for key, _ in pairs(wb) do
		table.insert(wb_key, key)
	end
	table.sort(wb_key)
	for _, key in pairs(wb_key) do
		only.log('D', string.format("[key:value][%s:%s]", 
		key, wb[key]))
		sign_str = sign_str ..key ..  wb[key]
	end
	sign_str = sign_str .. dianping_info['secret']
	only.log('D', string.format("[sign_str = %s]", sign_str))
	local result = sha.sha1(sign_str)
	local sign = string.upper(result)
	only.log('D', string.format("[sign:%s]", sign))
	return sign
end

local function get_coupon_cities()
	local wb = {}
	wb['sign'] = get_dianping_sign(wb)
	wb['appkey'] = dianping_info['appKey']
	local post_data = string.format(dianping_info['request'], 
	dianping_info['get_coupon_cities'], utils.gen_url(wb))
	--only.log('D', post_data)
	local ret = http_request(dianping_srv, post_data)
	if ret == nil then
		return nil
	end
	local result = string.match(ret, '{.+}')
	--only.log('D',string.format("[ok:%s]", ok))
	--only.log('D',string.format("[result:%s]", result))
	local ok, res = utils.json_decode(result)

	if not  ok then
		only.log('E', "decode result failed")
		return nil
	end
	if not res then
		only.log('E', "res is nil")
		return nil
	end
	local status = res['status']
	local cities = res['cities']
	if not status or status ~= 'OK' then
		only.log('D', 'dianping request is error')
		return nil
	end
	if not cities then
		only.log('D', "cities which get from dianping are nil")
		return nil
	end
	for i, v in pairs(cities) do
		only.log('D', string.format("[i:%s][v:%s]", i,v ))
	end
	return cities
end

local function get_business_categories()
	local wb = {}
	wb['sign'] = get_dianping_sign(wb)
	wb['appkey'] = dianping_info['appKey']
	local post_data = string.format(dianping_info['request'], 
	dianping_info['get_business_categories'], utils.gen_url(wb))
	--only.log('D', post_data)
	local ret = http_request(dianping_srv, post_data)
	if ret == nil then
		return nil
	end
	local result = string.match(ret, '{.+}')
	--only.log('D',string.format("[ok:%s]", ok))
	--only.log('D',string.format("[result:%s]", result))
	local ok, res = utils.json_decode(result)

	if not  ok then
		only.log('E', "decode result failed")
		return nil
	end
	if not res then
		only.log('E', "res is nil")
		return nil
	end
	local status = res['status']
	local categories = res['categories']
	if not status or status ~= 'OK' then
		only.log('D', 'dianping request is error')
		return nil
	end
	if not categories then
		only.log('D', "categories which get from dianping are nil")
		return nil
	end
	for i, v in pairs(categories) do
		only.log('D', string.format("[i:%s][v:%s]", i,v ))
	end
	return categories 
end
local function get_business_list(lon, lat, city, category)
	local accountID = pool["OUR_BODY_TABLE"]["accountID"]
	local cr_lon, cr_lat = map.correct_lonlat(lon, lat)
	local wb = {
		longitude = cr_lon,
		latitude = cr_lat,
		radius = 2000,
		category = category,
		platform = 2,
		--has_coupon = 1,
		sort = 7,
		limit = 20,
		--city= city,
	}
	wb['sign'] = get_dianping_sign(wb)
	wb['appkey'] = dianping_info['appKey']
	wb['category'] = utils.url_encode(wb['category'])
	--only.log('D', string.format("[wb:%s]", utils.gen_url(wb)))
	local post_data = string.format(dianping_info['request'], 
	dianping_info['get_business_list'], utils.gen_url(wb))
	local ret = http_request(dianping_srv, post_data)
	if ret == nil then
		return nil,nil
	end
	local result = string.match(ret, '{.+}')
	--only.log('D', string.format("[ret:%s]", result))
	local ok, res = utils.json_decode(result)
	if  not ok then
		only.log('D', 'decode error')
		return nil,nil
	end
	if not res then
		only.log('D', 'decode error')
		return nil,nil
	end
	--[[
	for i , v in pairs(res) do
	only.log('D', string.format('[%s:%s]', i, v))
	end
	--]]

	local status = res['status']
	if status ~= 'OK' then
		only.log('D', string.format('[dianping status is error]'))
		return nil,nil
	end
	local total_count = res['total_count']
	if tonumber(total_count) < 1 then
		only.log('D', string.format('[businesses cnt is less than 1]'))
		return nil,nil
	end
	local count = res['count']
	if tonumber(count) < 1 then
		only.log('D', string.format('[count is less than 1]'))
		return nil,nil
	end
	local businesses = res['businesses'] 
	if  businesses == nil then
		only.log('D', string.format('[businesses is nil]'))
		return nil, nil
	end

	--local business = nil
	--[[
	for i, v in pairs(businesses) do
	local coupon_id = v['coupon_id']
	local ok,res = redis_api.redis_apicmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sismember', 
	coupon_id_set_key, coupon_id)
	if not ok then return nil end;
	if not res then 
	redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', 
	coupon_id_set_key, coupon_id) 
	coupon = v
	break
	end
	end
	--]]

	return businesses, total_count
end

local function get_coupon_list(lon, lat, city,category)
	local accountID = pool["OUR_BODY_TABLE"]["accountID"]
	local coupon_id_set_key = accountID .. ':couponIDSet' 
	local cr_lon, cr_lat = map.correct_lonlat(lon, lat)
	local wb = {
		longitude = cr_lon,
		latitude = cr_lat,
		radius = 2000,
		category = category,
		sort = 5,
		limit = 20,
		city= city,
	}
	wb['sign'] = get_dianping_sign(wb)
	wb['appkey'] = dianping_info['appKey']
	wb['category'] = utils.url_encode(wb['category'])
	wb['city'] = utils.url_encode(wb['city'])
	only.log('D', string.format("[wb:%s]", utils.gen_url(wb)))
	local post_data = string.format(dianping_info['request'], 
	dianping_info['get_coupon_list'], utils.gen_url(wb))
	--only.log('D', string.format("[post_data:%s]",post_data))
	local ret = http_request(dianping_srv, post_data)
	--only.log('D', ret)
	if ret == nil then
		return nil,nil
	end
	local result = string.match(ret, '{.+}')
	--only.log('D', string.format("[ret:%s]", result))
	local ok, res = utils.json_decode(result)
	if  not ok then
		only.log('D', 'decode error')
		return nil,nil
	end
	if not res then
		only.log('D', 'decode error')
		return nil,nil
	end
	--[[
	for i , v in pairs(res) do
	only.log('D', string.format('[%s:%s]', i, v))
	end
	--]]

	local status = res['status']
	if status ~= 'OK' then
		only.log('D', string.format('[dianping status is error]'))
		return nil,nil
	end
	local total_count = res['total_count']
	if tonumber(total_count) < 1 then
		only.log('D', string.format('[coupon cnt is less than 1]'))
		return nil,nil
	end
	local count = res['count']
	if tonumber(count) < 1 then
		only.log('D', string.format('[count is less than 1]'))
		return nil,nil
	end
	local coupons = res['coupons'] 
	if coupons == nil then
		only.log('D', string.format('[coupons is nil]'))
		return nil,nil
	end

	--[[
	local coupon = nil
	for i, v in pairs(coupons) do
	local coupon_id = v['coupon_id']
	local ok,res = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sismember', 
	coupon_id_set_key, coupon_id)
	if not ok then return nil end;
	if not res then 
	redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', 
	coupon_id_set_key, coupon_id) 
	coupon = v
	break
	end
	end
	--]]

	return coupons, total_count
end

function handle()
	only.log('D', string.format("[--p2p_driving_pattern_remind--]"))
	--print("[p2p_driving_pattern_remind]")
	local wb = {}
	local idx_key = "drivingPatternRecognition"
	--[[
	--]]
	local accountID = pool["OUR_BODY_TABLE"]["accountID"]
	local lon       = pool["OUR_BODY_TABLE"]["longitude"][1]
	local lat       = pool["OUR_BODY_TABLE"]["latitude"][1]
	local speed     = pool["OUR_BODY_TABLE"]["speed"][1]
	local direction = pool["OUR_BODY_TABLE"]["direction"][1]
	local altitude  = pool["OUR_BODY_TABLE"]["altitude"] 
	and pool["OUR_BODY_TABLE"]["altitude"][1] 
	local PATTERN_STOP = 0
	local PATTERN_LOW_SPEED = 1
	local PATTERN_OVER_SPEED = 2
	local PATTERN_NORMAL = 3
	local pattern_tb = {
		[PATTERN_STOP] = PATTERN_STOP,
		[PATTERN_LOW_SPEED] = PATTERN_LOW_SPEED,
		[PATTERN_OVER_SPEED] = PATTERN_OVER_SPEED,
		[PATTERN_NORMAL] = PATTERN_NORMAL,
	}
	if accountID == nil then
		return false;
	end
	local ok, ret = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':' .. idx_key)
	if (not ok) or (not ret) then
		only.log('D', string.format("[ok is nil or ret is nil]"))
		return false
	end
	only.log('D', string.format("[ret = %s]", ret))
	--print(string.format("[ret = %s]", ret))

	local pattern, cur_speed, limit_speed =  string.match(ret, "(%w+):(%w+):(%w+)")
	if pattern == nil  then
		only.log('E', string.format("[pattern == nil]"))
		return false;
	end
	pattern = tonumber(pattern) or PATTERN_STOP 
	--only.log('D', string.format("[:%s]", ))

	if pattern ~= PATTERN_STOP and pattern ~= PATTERN_LOW_SPEED  and 
		pattern ~= PATTERN_OVER_SPEED and  pattern ~=  PATTERN_OVER_SPEED then
		only.log('E', ("[Wrong pattern no.]"))
		return false;
	end

	local text;
	local txt = {
		--[PATTERN_STOP] = '亲爱的%s,%s有优惠,%s,想要快按加键或加加键',
		--[昵称]，你是要在这停车吗？小道，为了找到了周边优惠券[张数]张，停车信息[个数]个，按加号键获取优惠券，按加加键获取停车场
		[PATTERN_STOP] = '%s，你是要在这停车吗？小道为你找到了周边优惠券%s张，停车信息%s个，按加号键获取优惠券，按加加键获取停车场。',
		[PATTERN_LOW_SPEED] = "哥们，开快点啊，你太慢了.",
		[PATTERN_OVER_SPEED] = "你当前的时速为%s,已超速",
		[PATTERN_NORMAL] = "常速巡航",
	}

	if pattern ~= PATTERN_STOP then
		return false
	end

	--- 正常条件
	if pattern ==  PATTERN_STOP then
		--For test
		--Data: LuJiaZui( lon = 121.50149876 ,lat = 31.23700579)
		--lon = 121.50149876 
		--lat = 31.23700579
		local nick_name = get_nickname(accountID)
		--print(string.format("[nickname:%s]", nick_name))
		--local city = "上海"

		local grid = string.format('%d&%d', tonumber(lon) * 100, tonumber(lat) * 100)
		only.log("D", "redis grid key:" .. grid)
		local ok, code_jo = redis_api.cmd('mapGridOnePercent',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', grid)
		if (not ok) or (not code_jo) then
			only.log("W", "can't get code from redis!")
			return false
		end
		only.log("D", "json data:" .. code_jo)
		local ok, code_tab = pcall(cjson.decode, code_jo)
		if not ok then
			only.log("E", "can't decode code! " .. code_tab)
			return false
		end
		local city_code = code_tab['cityCode']
		if not city_code then
			only.log("E", "can't get cityCode from json!")
			return false
		end
		local city = code_tab['cityName']
		if not city then
			only.log("E", "can't get cityName from json!")
			return false
		end
		city = string.gsub(city,"市","");

		if not dianping_info['cities'] then
			dianping_info['cities'] = get_coupon_cities()
		else
			--print("[dianping cities have beed existed.]")
		end
		local flag = false
		for i, v in pairs(dianping_info['cities']) do
			if v == city then
				flag = true
			end
		end
		if not flag then
			only.log('D', string.format("[city:%s] not supported coupon by dianping ", city))
			return 
		end
		only.log('D', string.format("[city:%s] supported coupon by dianping ", city))
		--local categories = get_business_categories()
		--only.log('D', scan.dump(categories))

		local business_list, total_count = get_business_list(lon, lat, city, '停车场')
		local business_info_count = total_count
		if  not business_list then
			return 
		end
		--[[
		for i, v in pairs(business_list) do
		only.log('D', scan.dump(v))
		end
		--]]


		local coupon_list, total_count = get_coupon_list(lon, lat, city,'美食')
		local coupon_info_count  = total_count
		if not coupon_list then
			return 
		end
		local coupon = coupon_list[1]
		--local coupon = coupon_list
		--local str = scan.dump(coupon)
		--only.log('D', string.format('[coupon_str:%s]', str))

		text = string.format(txt[PATTERN_STOP], 
		nick_name,coupon_info_count,business_info_count)
		--print(text)
		only.log('D', string.format("[text:%s]", text))
		--wb['text'] = text

		local openID = nil
		--openID = 'oBg_-jnBkALdiH8sGGnqfs39WoFI'
		local path = "/customizationapp/v2/callbackDrivingPatternRemind"
		local serv_callback =  link["OWN_DIED"]["http"][path]
		local bt = {
			openID  = openID,
			msgtype = "news",
			longitude = lon,
			latitude = lat,
			city = city,
			--title   = utils.url_encode(coupon['title']),
			--content = utils.url_encode(coupon['description']),
			--picurl  = utils.url_encode(coupon["logo_img_url"]),
			--url     = utils.url_encode(coupon["coupon_h5_url"]),
		}

		local callback_url  = "http://" .. serv_callback['host']  .. ':'  .. serv_callback['port']
		.. path .. "?" .. utils.gen_url(bt)

		only.log('D', string.format("[callback_url:%s]\n", callback_url))

		--wb['callbackURL'] =  utils.url_encode(callback_url)
		wb['callbackURL'] =  callback_url
	end

	--[[
	if pattern == PATTERN_LOW_SPEED then
	text = string.format(txt[PATTERN_LOW_SPEED])
	end
	if pattern ==  PATTERN_OVER_SPEED then
	text = string.format(txt[PATTERN_OVER_SPEED], cur_speed)
	wb['receiverSpeed'] = string.format("[%s]", limit_speed or 80)
	end
	if pattern == PATTERN_NORMAL then
	text = string.format(txt[PATTERN_NORMAL])
	end
	--]]

	--> send weibo
	local wb_url = {}
	wb_url['appKey'] = app_info["appKey"]
	wb_url['text'] = text
	local secret = app_info["secret"]
	local sign = utils.gen_sign(wb_url, secret)
	wb_url['sign'] = sign
	local serv = link["OWN_DIED"]["http"]["spx_txt_to_voice"]
	local req = utils.compose_http_kvps_request(serv, "spx_txt_to_voice", nil, wb_url, "POST")
	only.log('D', req)
	local fileurl
	local ok, resp = supex.http(serv["host"], serv["port"], req, #req)
	if not ok or not resp then
		only.log('D',string.format("fileurl is nil"))
		fileurl = nil
	end
	if ok and resp then
		only.log('D', resp)
		local jo = utils.parse_api_result( resp, "spx_txt_to_voice")
		fileurl =  jo and jo["url"] or nil
	end
	if fileurl then
		only.log('D',string.format("fileurl:%s",fileurl))
		--print(string.format("fileurl:%s",fileurl))
	else
		return false
	end

	wb['multimediaURL'] = fileurl
	wb['receiverAccountID'] = accountID
	wb['interval'] = 30
	wb['level'] = 30
	wb['content'] = text
	wb['senderType'] = 2
	--sourceId = fileID,
	if lon and lat then
		wb['senderLongitude'] = lon
		wb['senderLatitude'] = lat
	end
	wb["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
	wb["senderSpeed"] =  string.format('[%s]', speed and math.ceil(speed) or 0)
	if altitude then
		wb["senderAltitude"] =  altitude
	end
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok, ret = weibo_api.send_weibo( server, "personal", wb, app_info["appKey"], app_info["secret"] )
	if ok then
		local bizid = ret['bizid']
		local time = os.time()
		local travelID  = nil
		local appKey = app_info["appKey"]
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
	end

end
