local only = require('only')
local supex = require('supex')
local pool = require('pool')
local redis_pool_api = require('redis_pool_api')

local gosay = require('gosay')
local GPSPointModule = require("gps_point")
local MilPackageModule = require("mil_package")
local MilRecordModule = require("mil_record")
local mil_defs = require('mil_defs')


local GPSPoint = GPSPointModule.GPSPoint
local MilPackage = MilPackageModule.MilPackage
local MilRecord = MilRecordModule.MilRecord

module('mileage', package.seeall)


-- write lasted tokenCode to redis
local function write_tokenCode(imei, tokenCode)
	local key = imei .. ":tokenCode"
        local ok, ret = redis_pool_api.cmd('localRedis', '',  "SET", key, tokenCode)

        if not ok then
                only.log('E', string.format("write tokenCode error, key>>%s", key))
        end

	local ok, res = redis_pool_api.cmd('localRedis', '', "expire", key, 259200) -- ttl 72 hours
        if not ok or not res then
                only.log('E', string.format("reset TTL error, key>>%s", setKey))
        end
end

-- if tokenCode changed, add to the finish mileage sorted set, wait for the data curing
-- key	 > time:imei
-- value > imei:tokenCode:mileage
local function process_last_mileage(imei, accountID,tokenCode)
	if not imei or not tokenCode then
		return
	end

	local key = imei .. ":tokenCode"
	-- get last tokenCode from redis
        local ok, last_tokenCode = redis_pool_api.cmd('localRedis', '',  "GET", key)

	-- write lasted tokenCode to redis
	write_tokenCode(imei, tokenCode)
	if not ok or not last_tokenCode then
		return
	end
	
	-- add mileage to FINISH_MILE_ZSET
	local last_mileage_key = string.format("%s:%s:mileage", imei, last_tokenCode)
        local set_ok, _ = redis_pool_api.cmd(mil_defs['STATIC']['REDIS_NAME'], accountID or '',  "ZADD", 
		mil_defs['STATIC']['FINISH_MILE_ZSET'], os.time(), last_mileage_key)

	if not set_ok then
		only.log('D', string.format("key>>%s, add to FINISH_MILE_ZSET error", last_mileage_key))
	end

	-- delete this mileage from LASTED_MILE_ZSET
	local rem_ok, _ = redis_pool_api.cmd(mil_defs['STATIC']['REDIS_NAME'], accountID or '', "ZREM", 
				mil_defs['STATIC']['LASTED_MILE_ZSET'], last_mileage_key)

	if not rem_ok then
		only.log('D', string.format("key>>%s, rem from LASTED_MILE_ZSET error", last_mileage_key))
	end
end


local function process_mileage(milpkg)
        --check parameters
        local ok = milpkg:checkData()

        if not ok then 
                only.log('E','check parameter failed')
                return
	end

	if not milpkg['realFlag'] then
		local isOverTime = milpkg:isExpired()
			-- check package is Over the Time limit 
		 if isOverTime then
			only.log('D', "imei:" .. milpkg.imei .. "startTime:" .. milpkg['startTime']
				.. " , over time limit")
			return
		end
	end

        local logined = milpkg:checkExist()
        local milRcd = MilRecord:new()

        -- if the mileage calculation has not begin, close the last calculation by imei,
        if not logined then
		if milpkg['realFlag'] then
			process_last_mileage(milpkg['imei'], milpkg['accountID'], milpkg['tokenCode'])
		end

                milRcd:initFromData(milpkg)
                local mileage = milpkg:startCalc()
                milRcd:setNewData(mileage, milpkg['acptCount'], milpkg['normal'], milpkg['lowSpeedTime'], milpkg['stopTime'], milpkg['maxSpeed'], milpkg['speedSum'])

                milRcd:write()
        else
                local oldKey = milpkg['recordKey']
                milRcd.accountID = milpkg['accountID']
                milRcd:initFromRedis(oldKey)
                milRcd.realFlag = milpkg['realFlag']
                milRcd:updNewStart(milpkg['startTime'], milpkg['startLongitude'], milpkg['startLatitude'], milpkg['startDirection'])
                milRcd:updNewEnd(milpkg['endTime'], milpkg['endLongitude'], milpkg['endLatitude'], milpkg['endDirection'])
                local mileage = milpkg:startCalc()
                milRcd:setNewData(mileage, milpkg['acptCount'], milpkg['normal'], milpkg['lowSpeedTime'], milpkg['stopTime'], milpkg['maxSpeed'], milpkg['speedSum'])

                milRcd:update()
        end

	only.log('D', string.format("key>>%s, sumMileage:%.2f m, actualMileage:%.2f m", milRcd.recordKey, milRcd.sumMileageMts or 0, milRcd.actualMileageMts or 0 ))
end

function handle()
	--Get field from Body
	local req_body = pool["OUR_BODY_TABLE"]

        local milpkg = MilPackage:new()
        local ok = milpkg:init(req_body)

	if ok then
        	process_mileage(milpkg)
	end

        -- Init extragps data by request body
        extragps_body = req_body["extragps"]

	if extragps_body then
		local extragps_milpkg = MilPackage:new()
		local ok = extragps_milpkg:init(extragps_body)
		if not ok then
			return 
		end

		extragps_milpkg['realFlag'] = false
		extragps_milpkg['imei'] = milpkg['imei']
		extragps_milpkg['tokenCode'] = milpkg['tokenCode']
		extragps_milpkg['accountID'] = milpkg['accountID']
		extragps_milpkg['recordKey'] = milpkg['recordKey']
		extragps_milpkg['createTime'] = milpkg['createTime']

		process_mileage(extragps_milpkg)
	end
end

