local pool = require('pool')
local APP_POOL = require('pool')
local only = require('only')
local supex = require('supex')
local utils = require('utils')
local link = require('link')
local cjson = require('cjson')
local weibo_api = require('weibo')
--local redis_api = require('redis_short_api')
local redis_api = require('redis_pool_api')



module('p2p_fetch_4_miles_ahead_poi', package.seeall)


app_info = {
	appKey = "2491067261",
	secret = "52E8DCDEB8DBBAD220652851AE34339B008F5B48",
}

local function get_fileurl( text )
	local wb = {
		appKey = app_info['appKey'],
		text = text,
	}
	local secret = app_info['secret']
	local sign = utils.gen_sign(wb, secret)
	wb['sign'] = sign

	local serv = link["OWN_DIED"]["http"]["spx_txt_to_voice"]
	local req = utils.compose_http_kvps_request(serv, "spx_txt_to_voice", nil, wb, "POST")
	only.log('D', req)

	local ok, resp = supex.http(serv["host"], serv["port"], req, #req)
	if not ok or not resp then return nil end
	only.log('D', resp)

	local jo = utils.parse_api_result( resp, "spx_txt_to_voice")
	return jo and jo["url"] or nil
end

local function get_oil_price (lon, lat)
	local grid = string.format('%d&%d', tonumber(lon) * 100, tonumber(lat) * 100)
	only.log("D", "redis grid key:" .. grid)
	local ok, code_jo = redis_api.cmd('mapGridOnePercent01',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hgetall', grid)
	if (not ok) or (not code_jo) then
		only.log("W", "can't get code from redis!")
		return false
	end
	only.log('D', string.format("[code_jo = %s]", scan.dump(code_jo)))

	local provinceCode = code_jo['provinceCode']
	local cityCode = code_jo['cityCode']

	if not cityCode and not provinceCode then
		only.log("E", "can't get cityCode from redis!")
		return false
	end
	local wb_oil = {
		provinceCode = provinceCode,
		cityCode = cityCode,
	}
	local serv = link["OWN_DIED"]["http"]["/getOilPrice"]
	local req = utils.compose_http_kvps_request(serv, "getOilPrice", nil, wb_oil, "GET")
	only.log('D', string.format("req:%s", req))
	local ok, ret = supex.http(serv["host"], serv["port"], req, #req)
	only.log('D', string.format("[ok:%s][ret:%s]", ok, ret))
	--if not ok or not ret then return false end
	ret = string.match(ret, '{.+}')
	--ret = [[{"error":0,"result":["90号汽油:6.94","93号汽油:6.23","97号汽油:6.59","0号柴油:5.55"]}]]
	local ok, res = pcall(cjson.decode, ret) 
	if not ok then 
		only.log('W', "\r\nfailed to decode result")
		return false
	end  
	only.log('D', string.format("res:%s", scan.dump(res)))
	return res
end

function number_to_chinese(num)
	if not num then return '' end  
	if type(tonumber(num)) ~= "number" then return '' end  
	local num_list = {
		[0] = '零',
		[1] = '一',
		[2] = '二',
		[3] = '三',
		[4] = '四',
		[5] = '五',
		[6] = '六',
		[7] = '七',
		[8] = '八',
		[9] = '九' 
	}    
	local str = '' 
	for i = 1, #num do
		str = str .. num_list[tonumber(string.sub(num,i,i))]
	end  
	return str  
end


local function check_poi_type(poiType)
	if  poiType == 1123112 or 
		poiType == 1123114 or
		poiType == 1117102 or
		poiType == 1118101 or
		poiType == 1123106 or
		poiType == 1126157 then
		return true
	else
		return false
	end
end

function handle()
	local lon       = pool["OUR_BODY_TABLE"]["longitude"] and pool["OUR_BODY_TABLE"]["longitude"][1]
	local lat       = pool["OUR_BODY_TABLE"]["latitude"] and pool["OUR_BODY_TABLE"]["latitude"][1]
	local accountID = pool["OUR_BODY_TABLE"]["accountID"]
	local carry_key = accountID .. ':4MilesAheadPositionCarry'
	local idx_key = accountID .. ":4MilesAheadPositionTypeSet"
	--[[
	local res = get_oil_price (lon, lat)
	only.log('D', string.format("[lon&lat = %s]", scan.dump(res)))
	if res and res['result'] then
		for i , v in pairs(res['result']) do
			only.log('D', string.format("[v = %s]", v))
		end
	end
	if true then
		return false
	end
	--]]

	local ok, array = redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'smembers', idx_key)
	--only.log('D', string.format("array:%s", scan.dump(array)))
	if (not ok) or (not array) then
		return
	end
	local wb = {
		receiverAccountID = accountID,
		--level = 70, -- default
		level = 35, -- default
		senderType = 2,
	}

	local tb = {
		[111810201] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 动物园
		[111810202] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 动物园
                [111810203] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 动物园
                [111810204] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 动物园
                [111810205] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 动物园
                [111810206] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 动物园

		[111810301] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 植物园
		[111810302] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 植物园
		[111810303] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 植物园
                [111810304] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 植物园
                [111810305] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 植物园
                [111810306] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 植物园

		[112210601] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 驾校
		[112210602] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 驾校
		[112210603] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 驾校
                [112210604] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 驾校
                [112210605] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 驾校
                [112210606] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 驾校
                [112210607] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 驾校
                [112210608] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 驾校
        
		[112210201] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 中学
		[112210202] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 中学
                [112210203] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 中学
                [112210204] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 中学
                [112210205] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 中学
                
		[112210301] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 小学
		[112210302] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 小学
                [112210303] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 小学
                [112210304] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 小学
                [112210305] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 小学

		[112210401] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 幼儿园
		[112210402] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 幼儿园
                [112210403] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 幼儿园
                [112210404] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 幼儿园
                [112210405] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 幼儿园

		[111410101] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 农贸市场
		[111410102] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 农贸市场
                [111410103] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 农贸市场
                [111410104] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 农贸市场
                
                
		[112310101] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 隧道
		[112310102] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 隧道
		[112310103] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 隧道
                [112310104] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 隧道
                [112310105] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 隧道
                [112310106] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 隧道
                [112310107] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 隧道

		[112310701] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 大桥
		[112310702] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 大桥

		[112611101] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 事故易发地段
		[112611102] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 事故易发地段
                [112611103] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 事故易发地段
                [112611104] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 事故易发地段
                [112611105] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 事故易发地段

		--30,35,40,45,50,55,60,70,80,90,100,110,120
		[112311030] = 'http://g3.tweet.daoke.me/group3/M00/2E/58/rBALa1PSZIqAJFgkAAAKlobH0LQ178.amr', --前方有限速摄像头,限速30
		[112311040] = 'http://g3.tweet.daoke.me/group3/M01/2E/58/rBALaFPSZJCATGvqAAAKfMP-rkM788.amr', --前方有限速摄像头,限速40
		[112311035] = 'http://g3.tweet.daoke.me/group3/M00/2E/58/rBALaFPSZIyARcgKAAAK8aBt9t0088.amr', --前方有限速摄像头,限速35
		[112311045] = 'http://g3.tweet.daoke.me/group3/M01/2E/58/rBALa1PSZIqAI80nAAAK127sVtQ440.amr', --前方有限速摄像头,限速45
		[112311050] = 'http://g3.tweet.daoke.me/group3/M00/2E/59/rBALa1PSZI6AdRaBAAAKYjvsKuQ323.amr', --前方有限速摄像头,限速50
		[112311055] = 'http://g3.tweet.daoke.me/group3/M01/2E/58/rBALaFPSZImAGbvJAAAKsGOjj0A951.amr', --前方有限速摄像头,限速55
		[112311060] = 'http://g3.tweet.daoke.me/group3/M00/2E/58/rBALa1PSZIyATmN5AAAKfKOFhPk988.amr', --前方有限速摄像头,限速60
		[112311070] = 'http://g3.tweet.daoke.me/group3/M02/2E/59/rBALa1PSZI-AUSbDAAAKloL2QKs659.amr', --前方有限速摄像头,限速70
		[112311080] = 'http://g3.tweet.daoke.me/group3/M00/2E/58/rBALaFPSZJGAGsozAAAKfB8r1_0272.amr', --前方有限速摄像头,限速80
		[112311090] = 'http://g3.tweet.daoke.me/group3/M00/2E/59/rBALa1PSZJCAQhhhAAAKls3kutw430.amr', --前方有限速摄像头,限速90
		[1123110100] = 'http://g3.tweet.daoke.me/group3/M02/2E/59/rBALa1PSZI6ALHHNAAAKYmpukko543.amr', --前方有限速摄像头,限速100
		[1123110110] = 'http://g3.tweet.daoke.me/group3/M02/2E/58/rBALa1PSZIuAROt6AAALTN1Gmjw609.amr', --前方有限速摄像头,限速110
		[1123110120] = 'http://g3.tweet.daoke.me/group3/M02/2E/58/rBALa1PSZI2AD_HrAAALZmDfTGc974.amr', --前方有限速摄像头,限速120

		--[[
		[] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 
		--]]

		[112310501] = "http://g3.tweet.daoke.me/group3/M00/1B/5A/rBALa1O_PqWAe-OMAAAKsDdphhc050.amr", -- 加油站
                [112310502] = "http://g3.tweet.daoke.me/group3/M00/1B/5A/rBALa1O_PqWAe-OMAAAKsDdphhc050.amr", -- 加油站
                [112310503] = "http://g3.tweet.daoke.me/group3/M00/1B/5A/rBALa1O_PqWAe-OMAAAKsDdphhc050.amr", -- 加油站
                
		[112611201] = "http://g3.tweet.daoke.me/group3/M02/1B/94/rBALa1O_jeiALKEOAAAEMFVdWeY922.amr", --易滑
                [112611202] = "http://g3.tweet.daoke.me/group3/M02/1B/94/rBALa1O_jeiALKEOAAAEMFVdWeY922.amr", --易滑
                [112611203] = "http://g3.tweet.daoke.me/group3/M02/1B/94/rBALa1O_jeiALKEOAAAEMFVdWeY922.amr", --易滑
                [112611204] = "http://g3.tweet.daoke.me/group3/M02/1B/94/rBALa1O_jeiALKEOAAAEMFVdWeY922.amr", --易滑

		--[[
		[] = "http://g3.tweet.daoke.me/group3/M00/1A/DE/rBALaFO-ekeAZ5OwAAAJxm2AW7g006.amr", -- 
		--]]


		[1123111] = "http://g3.tweet.daoke.me/group3/M00/28/1C/rBALaFPNtCGAe5IpAAAG7lsi6S8177.amr", --前方有违章摄像头
		[112311101]="http",
		[112311102]="http",
		[112311103]="http",
		[112311001]="http",
		[112311002]="http",
		[1123110] = "http://g3.tweet.daoke.me/group3/M02/28/1C/rBALa1PNtCGAfOA-AAAHCHC93XM230.amr", --前方有限速摄像头
		[112611301] = "http://g3.tweet.daoke.me/group3/M01/1B/94/rBALaFO_jjmABF-fAAAHfb2HOZI947.amr", --村庄
                [112611302] = "http://g3.tweet.daoke.me/group3/M01/1B/94/rBALaFO_jjmABF-fAAAHfb2HOZI947.amr", --村庄
                [112611303] = "http://g3.tweet.daoke.me/group3/M01/1B/94/rBALaFO_jjmABF-fAAAHfb2HOZI947.amr", --村庄
                
		[1126116] = "http://g3.tweet.daoke.me/group3/M02/1B/93/rBALaFO_jFeACdlzAAAJAyTedUg707.amr", --有人看管的铁路道口
		[112611701] = "http://g3.tweet.daoke.me/group3/M02/1B/93/rBALaFO_jNWAaEaGAAAJA-W7jUg989.amr", --无人看管的铁路道口
                [112611702] = "http://g3.tweet.daoke.me/group3/M02/1B/93/rBALaFO_jNWAaEaGAAAJA-W7jUg989.amr", --无人看管的铁路道口
                
		[112611001] = "http://g3.tweet.daoke.me/group3/M00/1B/94/rBALaFO_jSCANFk3AAAFNJWPREA885.amr", --注意落石
                [112611002] = "http://g3.tweet.daoke.me/group3/M00/1B/94/rBALaFO_jSCANFk3AAAFNJWPREA885.amr", --注意落石
                
		[112611801] = "http://g3.tweet.daoke.me/group3/M00/1B/95/rBALaFO_jsWALOmFAAAH2N9kD4U799.amr", --道路变窄
                [112611802] = "http://g3.tweet.daoke.me/group3/M00/1B/95/rBALaFO_jsWALOmFAAAH2N9kD4U799.amr", --道路变窄
                
		[112611901] = "http://g3.tweet.daoke.me/group3/M01/1B/96/rBALa1O_jw6AA2w1AAAIqFKIz0M925.amr", --向左急弯路
                [112611902] = "http://g3.tweet.daoke.me/group3/M01/1B/96/rBALa1O_jw6AA2w1AAAIqFKIz0M925.amr", --向左急弯路
                [112611903] = "http://g3.tweet.daoke.me/group3/M01/1B/96/rBALa1O_jw6AA2w1AAAIqFKIz0M925.amr", --向左急弯路
                
		[112612001] = "http://g3.tweet.daoke.me/group3/M01/1B/96/rBALa1O_j3OAZrYjAAAIjvKy1NI080.amr", --向右急弯路
                [112612002] = "http://g3.tweet.daoke.me/group3/M01/1B/96/rBALa1O_j3OAZrYjAAAIjvKy1NI080.amr", --向右急弯路
                [112612003] = "http://g3.tweet.daoke.me/group3/M01/1B/96/rBALa1O_j3OAZrYjAAAIjvKy1NI080.amr", --向右急弯路
                
                
		[112612201] = "http://g3.tweet.daoke.me/group3/M02/2D/D2/rBALaFPSDnWAAXyWAAAJN3Gbo3M979.amr", -- 连续转弯路段，请注意盲区
                [112612202] = "http://g3.tweet.daoke.me/group3/M02/2D/D2/rBALaFPSDnWAAXyWAAAJN3Gbo3M979.amr", -- 连续转弯路段，请注意盲区
                
		[112612301] = "http://g3.tweet.daoke.me/group3/M01/2D/D2/rBALa1PSDnaADWZ0AAAHLxUv5Dc832.amr", -- 请注意左侧并线车辆。
                [112612302] = "http://g3.tweet.daoke.me/group3/M01/2D/D2/rBALa1PSDnaADWZ0AAAHLxUv5Dc832.amr", -- 请注意左侧并线车辆。
                
		[112612401] = "http://g3.tweet.daoke.me/group3/M00/2D/D2/rBALaFPSDnaARBDPAAAHL1ouDrQ179.amr", -- 请注意右侧并线车辆。
                [112612402] = "http://g3.tweet.daoke.me/group3/M00/2D/D2/rBALaFPSDnaARBDPAAAHL1ouDrQ179.amr", -- 请注意右侧并线车辆。
                
		[112615901] = 'http://127.0.0.1/productList/POIRemind/1126159.amr', --前方有右转红灯，请注意观察
                [112615902] = 'http://127.0.0.1/productList/POIRemind/1126159.amr', --前方有右转红灯，请注意观察
                
		[112616001] = 'http://127.0.0.1/productList/POIRemind/1126160.amr', --前方有右转避让行人，请谨慎驾驶
                [112616002] = 'http://127.0.0.1/productList/POIRemind/1126160.amr', --前方有右转避让行人，请谨慎驾驶
	}

	local amr_host  = link["OWN_DIED"]["http"]["4milesAMR"]["host"]
	local fmt = "http://%s/productList/POIRemind/%s.amr"
	for i, v in pairs(tb) do
		tb[i] = string.format(fmt, amr_host, i)
	end

	only.log('D', string.format('[carry_key = %s]', scan.dump(carry_key)))
	only.log('D', string.format('[carry = %s]', scan.dump(array)))

	for _,idx in pairs(array or {}) do
		if carry_key then
			local ok, info = redis_api.cmd('driview',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', carry_key, tostring(idx))
			only.log('D', string.format("[ok:%s][info:%s]", ok, scan.dump(info)))
			if ok and info then
				local ok,jo = pcall(cjson.decode, info)
				if ok and jo then
					only.log('D', string.format("[ok:%s][jo:%s]", ok, scan.dump(jo)))
					for k,v in pairs(jo) do
						only.log('D',string.format("%s:%s", k, v))
						if tostring(k) == "context" then
							wb[ 'content' ] = v
							only.log('D',string.format("idx:%s", idx))

							if tonumber(idx) == 1123105  then
								local res = get_oil_price(lon, lat)
								if res and res['error'] == 0 and res['result'] and res['result'] ~= '' then
									wb[ 'content' ] = wb[ 'content' ] .. ',本市今日油价,'
									for i, v in pairs(res['result']) do
										local a,b = string.match(v, '(.+%.)(.+)')
										a = string.gsub(a, '%.', '点')
										b = number_to_chinese(b)
										wb[ 'content' ] = wb[ 'content' ] .. ',' .. a .. b .. '元'
									end
								end
								only.log('D', string.format('content:%s', wb['content']))
								if v ~= wb[ 'content' ] and tb[tonumber(idx)]  then
									tb[tonumber(idx)] = nil
								end
							end

								--[[
							if  tonumber(idx) == 1123106 then
								local res = get_oil_price(lon, lat)
								if res and res['error'] == 0 and res['result'] and res['result'] ~= '' then
									wb[ 'content' ] = wb[ 'content' ] .. ',本市今日油价,'
									for i, v in pairs(res['result']) do
										local a,b = string.match(v, '(.+%.)(.+)')
										a = string.gsub(a, '%.', '点')
										b = number_to_chinese(b)
										wb[ 'content' ] = wb[ 'content' ] .. ',' .. a .. b .. '元'
									end
								end
								only.log('D', string.format('content:%s', wb['content']))
								if v ~= wb[ 'content' ] and tb[tonumber(idx)]  then
									tb[tonumber(idx)] = nil
								end
							end
								--]]

                                                        --是否依据ＰＯＩＩＤ下发的
							if check_poi_type(tonumber(idx)) then
								wb["multimediaURL"] = string.format(fmt, amr_host, v)
							else
								if tb[tonumber(idx)] then --- 固定
									wb["multimediaURL"] = tb[tonumber(idx)]
								else
									local context = wb[ 'content' ];
									local fileurl = get_fileurl(context)
									--only.log('D', string.format("[fileurl]", fileurl))
									wb["multimediaURL"] = fileurl
								end
							end
						else
							wb[ k ] = v
						end -- tostring(k)
					end -- for jo
				end -- ok jo
			end -- ok info
		end -- carry_key

		--1126111
		local position_type = string.sub(idx,0,7)  
		local url_tb  = {
			POILongitude    = wb["receiverLongitude"],
			POILatitude     = wb["receiverLatitude"],
			POIDirection    = wb["receiverDirection"],
			POIContent      = wb["content"],
			positionType    = position_type,
			POIID           = nil,
			roadID          = nil,
		}

		local path = "/customizationapp/v2/callbackFetch4MilesAheadPoi"
		local callback_serv = link["OWN_DIED"]["http"][path] 
		local callback_url = utils.gen_url(url_tb)
		wb['callbackURL'] =  "http://" .. callback_serv['host'] .. ":" .. callback_serv['port'] 
		.. path .. "?" .. callback_url 

		local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
		for i, v in pairs(wb or {}) do
			only.log("D", string.format("[%s:%s]", i, v))
		end
		if wb["multimediaURL"] then
			local ok,ret = weibo.send_weibo( server, "personal", wb, app_info['appKey'], app_info['secret'] )
			if ok then
				--[[
				local bizid = ret['bizid']
				local time = os.time()
				local travelID  = nil
				local appKey = position_type
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
				--]]
			else
				only.log("E", "send weibo failed : " .. err)
			end
		end
	end
end
