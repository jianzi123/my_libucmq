local pool = require('pool')
local only = require('only')
local supex = require('supex')
local utils = require('utils')
local link = require('link')
local cjson = require('cjson')

module('w_traffic_remind', package.seeall)

local function get_direction_describe( dir )
	local direction
	local d = tonumber( dir )
	if d<80 and d>10 then
		direction = '由西南向东北'
	elseif d<=100 and d>=80 then
		direction = '由西向东'
	elseif d<170 and d>100 then
		direction = '由西北向东南'
	elseif d<=190 and d>=170 then
		direction= '由北向南'
	elseif d<260 and d>190 then
		direction = '由东北向西南'
	elseif d<=280 and d>=260 then
		direction = '由东向西'
	elseif d<350 and d>280 then
		direction = '由东南向西北'
	elseif (d<=10 and d>=0) or (d>=350 and d<=360) then
		direction = '由南向北'
	end
	return direction
end

local function get_mapabc_traffic( lon, lat, dir, speed )
	--[[
	local fmt = 'GET %s?config=trafficinfo&datatype=0&gpsdata=%f,%f,%f,%f;&flag=62&userName=%s&pcode=%s HTTP/1.0\r\nHost: %s\r\n\r\n'
	local req = string.format(fmt, '/traffic', lat, lon, math.floor(speed), math.floor(dir), 'yujing', 'MDAwMDAw', 'online.mapabc.com')
	]]--

	local req = utils.compose_http_kvps_request({host = 'online.mapabc.com'}, "traffic", nil,
	{
		config = "trafficinfo",
		datatype = 0,
		gpsdata = string.format("%f,%f,%f,%f;", lat, lon, math.floor(speed), math.floor(dir)),
		flag = 62,
		userName = 'yujing',
		pcode = 'MDAwMDAw'
	},
	"GET")
	only.log('D', req)

	local ok, resp = supex.http('online.mapabc.com', 80, req, #req)
	if not ok or not resp then return nil end
	only.log('D', resp)
	
	-- <![CDATA[ ... ]]>
	local traffic = string.match(resp, '%!%[CDATA%[(.+)%]%]')
	if not traffic then return nil end
	only.log('D', traffic)

	-- traffic = "当前行驶角度，" .. mapabc_direct .. "度,"  .. string.sub(resp, begin_pos + 6, end_pos - 1)
	return traffic
end

local function get_dir_allow_space(base_val, change_val)
	local tab_space = {}
	if base_val > 0 and base_val < change_val then
		tab_space[1] = {0, base_val + change_val}
		tab_space[2] = {360 - (change_val - base_val), 360}
	elseif base_val > (360 - change_val ) and base_val < 360 then
		tab_space[1] = {0, base_val + change_val - 360}
		tab_space[2] = {base_val - change_val, 360}
	elseif base_val == 0 then
		tab_space[1] = {0, change_val}
		tab_space[2] = {360 - change_val, 360}
	else
		tab_space[1] = {base_val - change_val , base_val + change_val}
	end
	return tab_space
end

local function check_dir_is_allow(tab_space, check_val)
	if #tab_space == 1 then
		if check_val < tab_space[1][1] or check_val > tab_space[1][2] then
			return false
		end
	else
		if (check_val < tab_space[1][1] or check_val > tab_space[1][2]) and
			(check_val < tab_space[2][1] or check_val > tab_space[2][2]) then
			return false
		end
	end
	return true
end

local function send_weibo(accountID, urls, content, lv)
	local wb = {
		appKey = "2328738433",
		multimediaURL = urls,
		receiverAccountID = accountID,
		interval = 40,
		level = lv,
		content = content,
		senderType = 2,   ---------添加发送类型区分微博来源 1:WEME    2:system    3:other
	}
	local secret = "7349A77C48504E61FAEDA521B5C85ECFF04BCFCB"
	local sign = utils.gen_sign(wb, secret)
	wb['sign'] = sign

	local boundary = '---------------YouAreAngel'
	local serv = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local post_body = utils.format_http_form_data(wb, boundary)
	local post_head = 'POST /weiboapi/v2/sendMultimediaPersonalWeibo HTTP/1.0\r\n' ..
	'HOST:' .. serv['host'] .. ":" .. tostring(serv['port']) .. '\r\n' ..
	'Content-Length:' .. tostring(#post_body) .. '\r\n' ..
	'Content-Type:multipart/form-data; boundary=' .. boundary .. '\r\n\r\n'

	local post_data = post_head .. post_body
	only.log('D',"post data : " .. post_data)
	local ok, ret = supex.http(serv['host'], serv['port'], post_data, #post_data)
	only.log('D', ret)
end

local function get_fileurl( text )
	local wb = {
		appKey = "2328738433",
		text = text,
	}
	local secret = "7349A77C48504E61FAEDA521B5C85ECFF04BCFCB"
	local sign = utils.gen_sign(wb, secret)
	wb['sign'] = sign

	local serv = link["OWN_DIED"]["http"]["dfsapi/v2/txt2voice"]
	local req = utils.compose_http_kvps_request(serv, "dfsapi/v2/txt2voice", nil, wb, "POST")
	only.log('D', req)

	local ok, resp = supex.http(serv["host"], serv["port"], req, #req)
	if not ok or not resp then return nil end
	only.log('D', resp)
	
	local body = string.match(resp, '{.*}')
	local ok,jo = pcall(cjson.decode, body)
	if not ok or not jo then
		only.log('E', jo)
		return nil
	end
	if tonumber(jo["ERRORCODE"]) ~= 0 then return nil end
	local fileurl = jo["RESULT"]["url"]
	if not fileurl then return nil end
	only.log('D', fileurl)


	return fileurl
end

function handle()
	-->-->-->-->-->-->-->-->-->-->
	local gps_data = pool["OUR_BODY_TABLE"]

	local current_longitude = gps_data["longitude"][1]
	local current_latitude = gps_data["latitude"][1]
	local valid_direction = nil
	local valid_speed = nil
	local is_direction_changed = false

	--> get a valid package, -1 is bad
	for k,v in ipairs(gps_data["direction"] or {}) do
		if v ~= -1 then
			valid_direction = v
			valid_speed = gps_data["speed"][ k ]
			break
		end
	end
	
	only.log('D', "START=====")
	if valid_direction and valid_direction ~= -1 then
		local tab_dir_space_change = get_dir_allow_space(valid_direction, 70)

		local after = -1
		for k,v in ipairs(gps_data["direction"]) do
			only.log('D', "[" .. v .. "]")

			if v ~= -1 then
				if  after ~= -1 then
					--> check if is bad packet
					local tab_dir_space_between = get_dir_allow_space( after, 150 )

					local is_allowed = check_dir_is_allow( tab_dir_space_between, v )
					if not is_allowed then
						only.log('W', "(is bad packet)")
						break
					end

				end

				local no_changed = check_dir_is_allow( tab_dir_space_change, v )
				if not no_changed then
					only.log('I', "(is true)")
					is_direction_changed = true
					break
				end
			end
			after = v
		end
	end
	only.log('D', "ENDING=====")

	if is_direction_changed then
		-->> direction
		local direction_text = get_direction_describe( valid_direction )
		local content = string.format("当前时速%d千米每小时,行驶方向%s。" , valid_speed, direction_text)
		local fileurl = get_fileurl( content )
		send_weibo(gps_data["accountID"], fileurl, content, 30)
		
		-->> traffic
		local info = get_mapabc_traffic( current_longitude, current_latitude, valid_direction, valid_speed )
		if info then
			local fileurl = get_fileurl( info .. ",来自高德路况。" )
			send_weibo(gps_data["accountID"], fileurl, info, 30)
		end
	end
end
