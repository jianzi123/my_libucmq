-- Auther:GengXuanxuan
-- Date:2014-04-13
-- Feature:#2267
--
-- GPS Data package class

local GPSPointModule = require("gps_point")
local utils = require("utils")
local GPSPoint = GPSPointModule.GPSPoint
local mil_defs = require('mil_defs')

module("mil_package", package.seeall)

-- Mileage record that map hash objects of redis
MilPackage = {
        realFlag = true,
        logined = false,
}

-- Return new object to MilPackage
MilPackage.new = function(self)
        local attr = {
                
        }

        setmetatable(attr, self)
        self.__index = self

        return attr
end

-- Init MilPackage object by request body
MilPackage.init = function(self, req_body)
	self['imei']        = req_body['IMEI']
	self['tokenCode']    = req_body['tokenCode']
	self['accountID']    = req_body['accountID']
	self['createTime']   = os.time()
	self['normal'] = nil
	self['lowSpeedTime'] = 0
	self['stopTime'] = 0
	self['maxSpeed'] = 0
	self['speedSum'] = 0
	self['acptCount'] = 0


	if self['imei'] and self['tokenCode'] then
		self['recordKey'] = self['imei'] .. ":" .. self['tokenCode'] .. ":mileage"
	end

        local tmpPoints = {}
        local dataBeginTime = req_body['GPSTime'][1]
	local dataEndTime = dataBeginTime
        for key,val in ipairs(req_body['GPSTime']) do
                local ptTime = req_body['GPSTime'][key]
                local tmp = {}

                tmp['GPSTime'] = ptTime
                tmp['longitude'] = req_body['longitude'][key]
                tmp['latitude'] = req_body['latitude'][key]
                tmp['speed'] = req_body['speed'][key]
                tmp['direction'] = req_body['direction'][key]

                tmpPoints[ptTime] = tmp

		--获得本次开始和结束时间
		if ptTime < dataBeginTime then
			dataBeginTime = ptTime
		elseif ptTime > dataEndTime then
			dataEndTime = ptTime
		end

        end

        self['startTime'] = dataBeginTime
        self['endTime'] = dataEndTime

        self['startLongitude'] = tmpPoints[dataBeginTime]['longitude']
        self['startLatitude'] = tmpPoints[dataBeginTime]['latitude']
        self['startDirection'] = tmpPoints[dataBeginTime]['direction']
        self['endLongitude'] = tmpPoints[dataEndTime]['longitude']
        self['endLatitude'] = tmpPoints[dataEndTime]['latitude']
        self['endDirection'] = tmpPoints[dataEndTime]['direction']
	self['GPSPoints'] = tmpPoints

--	if req_body['GPSTime'][1] then
--		self['acptCount'] = #(req_body['GPSTime'])
--	end

	--获得本次异常停车标志
	if self['GPSPoints'][dataEndTime]['speed'] < 5 then
		self['normal'] = 1
	else
		self['normal'] = 0
	end

	return true
end

-- Check the integrity of data
MilPackage.checkData = function(self)
        if not  utils.check_imei(self['imei']) then
                only.log('E', "check imei fail")
                return false
        end

        if not self['tokenCode'] then
                only.log('E', "tokenCode is nil")
                return false
        end

	if not self['startTime'] then
                only.log('E', "startTime is nil")
                return false
	end

	if not self['endTime'] then
                only.log('E', "endTime is nil")
                return false
	end
	
--	if self['acptCount'] < 1 then
--                only.log('E', "no GPS data")
--                return false
--	end

        return true
end

-- Check the hash key exist in Redis
MilPackage.checkExist = function(self)
        local ret = false
        local ok, res = redis_pool_api.cmd(mil_defs['STATIC']['REDIS_NAME'], self['accountID'] or '', "EXISTS", self['recordKey'])

        if ok and res then
                ret = true
        else
                ret = false
        end

        self['logined'] = ret

        return ret
end

-- package create time is over the time limit, (1 hour and 10 minutes)
-- if true, pass the calculation
MilPackage.isExpired = function(self)
        local curTime = os.time()
        if (curTime - self['startTime']) > mil_defs['STATIC']['TIME_LIMIT'] then
                return true
        else
                return false
        end
end

-- write Lasted Point to Redis(include GPSTime and speed)
-- Redis Type: string
-- key: lastpoint:imei:token
-- value: GPSTime:speed
local function writeLastPoint(imei,tokenCode, GPSTime, speed, direction)
        local lastPointKey = imei .. ":" .. tokenCode .. ":lastpoint"
        local val = GPSTime .. ":" .. speed .. ":" .. direction

        local ok,_ = redis_pool_api.cmd('localRedis', '', "SET", lastPointKey, val)

        if not ok then
                only.log('E', string.format("write last point error, key>>%s",
                        lastPointKey))
        end

        -- reset TTL
        local ok, res = redis_pool_api.cmd('localRedis', '', "expire", lastPointKey, mil_defs['STATIC']['TIME_LIMIT'])
        if not ok or not res then
                only.log('E', string.format("reset TTL error, key>>%s", setKey))
        end
end

-- Get Lasted GPS Point From Redis(include GPSTime and speed)
local function readLastPoint(imei, tokenCode)
        local lastPointKey = imei .. ":" .. tokenCode .. ":lastpoint" 

        local ok, ret = redis_pool_api.cmd('localRedis', '',  "GET", lastPointKey)

        if (not ok ) or ( not ret) then
                only.log('D', string.format("no last point or last point over time limit, key>>%s",
                        lastPointKey))
                return 
        end

        local sPos1, ePos1 = string.find(ret, ":")
        local GPSTime = tonumber(string.sub(ret, 1, sPos1 - 1))

        local sPos2, ePos2 = string.find(ret, ":", ePos1 + 1)
        local speed = tonumber(string.sub(ret, ePos1 + 1, sPos2 - 1))
        local direction = tonumber(string.sub(ret, ePos2 + 1))

        return  GPSTime, speed, direction
end


-- write loss points (between previous and next point,not include this two points) 
--      to redis, type is sorted set
-- key: "losspoint:imei"
-- score : GPSTime
-- value: LossPointTime:prevTime:prevSpeed:nextTime:nextSpeed
--      prevTime is the last GPSTime before loss time area,
--      prevSpeed is the speed of last point before loss time area,
--      nextTime is the first GPSTime after the loss time area,
--      nextSpeed is the speed of first point after loss time area
-- reset the TTL to TIME_LIMIT (1 hour and 10 minutes)
local function writeLossPoints(imei, tokenCode, prevPt, nextPt)
        local setKey = imei .. ":" .. tokenCode .. ":losspoint"
        local args = {}
	local UNPACK_MAX = mil_defs['STATIC']['UNPACK_MAX']
        
        local prevTime = prevPt['GPSTime']
        local nextTime = nextPt['GPSTime']
        local itvl = nextTime - prevTime

	if itvl > UNPACK_MAX then 
		only.log('W', string.format("imei:%s, tokenCode:%s, write large loss points, NUM:%d", imei, tokenCode, itvl))
	end

        local valueStr = prevPt['GPSTime'] .. ":" .. prevPt['speed'] .. ":" 
                        .. nextPt['GPSTime'] .. ":" .. nextPt['speed'] 

	local beginTime = prevTime + 1
	local endTime = nextTime - 1
	local index = 0
	local MAX = UNPACK_MAX
	local points_num = endTime - beginTime

	for timeVal = beginTime, endTime do
                local tmpStr = timeVal .. ":" .. valueStr 
                table.insert(args, timeVal)
                table.insert(args, tmpStr)
		index = index + 1
		
		if (index % MAX) == 0 then 
			local ok, res = redis_pool_api.cmd('localRedis', '',  "ZADD", setKey, unpack(args))
			args = {}

			if not ok then
				only.log('E', string.format("write loss points error, key>>%s", setKey))
			end
		end
	end

	if #args ~= 0 then
	        local ok, res = redis_pool_api.cmd('localRedis', '',  "ZADD", setKey, unpack(args))

		if not ok then
			only.log('E', string.format("write loss points error, key>>%s", setKey))
			return
		end
	end

        --reset TTL to setKey
        local ok, res = redis_pool_api.cmd('localRedis', '', "expire", setKey, mil_defs['STATIC']['TIME_LIMIT'])
        if not ok or not res then
                only.log('E', string.format("reset TTL error, key>>%s", setKey))
        end
end

-- Get loss point by beginTime and endTime (include this two points)
--      , sorted set key :"losspoint:imei:tokenCode"
-- return array of GPSTime
-- key: "losspoint:imei"
-- score : GPSTime
-- value: LossPointTime:beginTime:beginSpeed:endTime:endSpeed
--      beginTime is the last GPSTime before loss time area,
--      beginTime is the speed of last point before loss time area,
--      endTime is the first GPSTime after the loss time area,
--      endSpeed is the speed of first point after loss time area
-- return Points:
--      GPSTime
--      beginTime
--      beginSpeed
--      endTime
--      endSpeed
local function readLossPoints(imei, tokenCode, beginTime, endTime)
        local itvl = endTime - beginTime

        local setKey = imei .. ":" .. tokenCode .. ":losspoint"
        local ok,res = redis_pool_api.cmd('localRedis', '', "ZRANGEBYSCORE", 
                               setKey, beginTime ,endTime)

        if not ok or not res then
                only.log('D', string.format("no loss points or loss points over time limit , key>>%s", setKey))
                return {}
        end

        local retPoints = {}

        for k, val in ipairs(res) do
        repeat
                local info = utils.str_split(val, ":")

                if not info then
                        only.log('E', string.format("readLossPoints,split (%s) error", val))
                        break -- continue
                end

                local pt = {}
                retPoints[k] = pt

                pt['GPSTime'] = tonumber(info[1])
                pt['beginTime'] = tonumber(info[2])
                pt['beginSpeed'] = tonumber(info[3]) 
                pt['endTime'] = tonumber(info[4])
                pt['endSpeed'] = tonumber(info[5])
        until true
        end

        return retPoints
end

--  delete the points from beginTime to endTime from loss area points in redis
local function delLossPoints(imei, tokenCode, beginTime, endTime)
        local setKey = imei .. ":" .. tokenCode .. ":losspoint"

        local ok,res = redis_pool_api.cmd('localRedis', '', "ZREMRANGEBYSCORE", 
                                setKey, beginTime, endTime)

        if not ok or not res then
                only.log('E', string.format("delLossPoints error, key>>%s", setKey))
        end
end

-- delete the over time(4200 second ago) lossing in from redis
local function delLossPointsOverTime(imei, tokenCode, beginTime, endTime)
	local curTime = os.time()
	local closeEnd = curTime - mil_defs['STATIC']['TIME_LIMIT']

	if beginTime < closeEnd then
		closeEnd = beginTime
	end

        local setKey = imei .. ":" .. tokenCode .. ":losspoint"

        local ok,res = redis_pool_api.cmd('localRedis', '', "ZREMRANGEBYSCORE", 
                                setKey, 0, closeEnd)
        if not ok then
                only.log('E', string.format("delLossPointsOverTime error, key>>%s", setKey))
        end
end

local function sort_func(a, b) 
	return a['GPSTime'] < b['GPSTime']
end

-- Get real time package gps points
MilPackage.getRealPoints = function(self)
        local points = {}

        if not self['realFlag'] then
                only.log('E', string.format("MilPackage.getRealPoints, not real data,key>>%s", self['recordKey']))
                return nil
        end
	
        -- if not logined, the first gps point is the lasted point
        for key,val in pairs(self['GPSPoints']) do
                local p = GPSPoint:new()
                p:init{GPSTime=key,speed=self['GPSPoints'][key]['speed'],direction=self['GPSPoints'][key]['direction']}

                table.insert(points,p)
        end

        -- if logined, get lasted point from redis
        if self['logined'] then
                local lastGPSTime, lastSpeed, lastDirection = readLastPoint(self['imei'], self['tokenCode']) 

                if not lastGPSTime then
                        return points
                end

                local p = GPSPoint:new()
                p:init{GPSTime=lastGPSTime,speed=lastSpeed,direction=lastDirection}

                if lastGPSTime < self['startTime'] and (self['startTime'] - lastGPSTime) < mil_defs['STATIC']['TIME_LIMIT'] then
			points['repetition'] = true	--上一个数据包中的点
                        table.insert(points, 1, p)
                end
        end

	-- the gps data may be unsorted
	table.sort(points, sort_func)

        return points
end

MilPackage.calcMil = function(self, points)
        local n = table.getn(points)

        if n < 2 then
                return 0
        end

	self['acptCount'] = n

        local mileage = 0
        for var = 1,n do
        repeat
		if not points[var]['repetition'] then
			--获得本次低速
			if points[var]['speed'] < 20 and points[var]['direction'] ~= -1 then
				self['lowSpeedTime'] = self['lowSpeedTime'] + 1
			end 
	
			--获得本次停车时间
			if points[var]['direction'] == -1 then
				self['stopTime'] = self['stopTime'] + 1
			end
	
			--获得本次最大速度
			if points[var]['speed'] > self['maxSpeed'] then
				self['maxSpeed'] = points[var]['speed']
			end
	
			--获得本次速度和
			self['speedSum'] = self['speedSum'] + points[var]['speed']
		end

		if var == 1 then
			break	--第一个点 continue
		end
	

                local itvl = points[var].GPSTime - points[var-1].GPSTime

                local speed1 = ((points[var-1]['direction'] == -1) and 0) or points[var-1]['speed']
                local speed2 = ((points[var]['direction'] == -1) and 0) or points[var]['speed']
	
                if itvl > mil_defs['STATIC']['TIME_INTERVAL_MAX'] then
                        local prevPt = {}
                        local nextPt = {}

                        prevPt['GPSTime'] = points[var-1]['GPSTime'] 
                        prevPt['speed'] = speed1
                        nextPt['GPSTime'] = points[var]['GPSTime']
                        nextPt['speed'] = speed2

                        writeLossPoints(self['imei'], self['tokenCode'], prevPt, nextPt)
                        break   -- continue the for loop
                elseif itvl < 1 then
                        break   -- continue the for loop
                end

                local tmpMil = ((speed1 + speed2) / 2 ) * ( 1000 / 3600 ) * itvl
                tmpMil = tmpMil - tmpMil%0.01  -- format %.2f
                mileage = mileage + tmpMil
        until true
        end

        return mileage

end

-- Get additional data points where not in loss points of redis
MilPackage.getAddPoints = function(self)
        local points = {}

        if self['realFlag'] then
                only.log('E', string.format("MilPackage.getAddPoints, not additional data:%s", self['recordKey']))
                return nil
        end
        
        -- insert the loss point to table
        -- (speed and direction information from additional data)
        -- convert additional data,
        -- key:GPSTime
        -- value: GPSPoint table
        local tmpPoints = self['GPSPoints']
        local dataBeginTime = self['startTime']
	local dataEndTime = self['endTime']

	if not dataBeginTime or not dataEndTime then
		return nil
	end

        -- Get loss Point from Redis
        local lossPoints = readLossPoints(self['imei'], self['tokenCode'],dataBeginTime, dataEndTime)

        local nLosPt = table.getn(lossPoints)

        if nLosPt < 1 then
                return nil
        end

        local lossBeginTime = lossPoints[1]['beginTime']
        local lossEndTime = lossPoints[nLosPt]['endTime']

        -- Get Additional calculation points
        local firstPoint = GPSPoint:new()
        firstPoint:init{GPSTime=lossBeginTime, speed=lossPoints[1]['beginSpeed'], direction=9999}	--direction=9999，means the direction is not -1

        -- Get the end point and coincide area to table
        local lastPoint = GPSPoint:new()
        lastPoint:init{GPSTime=lossEndTime, speed=lossPoints[nLosPt]['endSpeed'], direction=9999}

        if firstPoint then
		firstPoint['repetition'] = true	--上一个数据包中的点
                table.insert(points, firstPoint) 
        end

        -- insert the coincide points into table
        for key,val in pairs(lossPoints) do
                local ptTime = val['GPSTime']

                if tmpPoints[ptTime] then
                        local pt = GPSPoint:new()
                        pt:init{GPSTime=ptTime, speed=tmpPoints[ptTime]['speed'], 
                                direction=tmpPoints[ptTime]}
                        table.insert(points,pt)
                end

        end

        -- insert the last Point to table
        if lastPoint then
		lastPoint['repetition'] = true	--上一个数据包中的点
                table.insert(points, lastPoint) 
        end

	-- the gps data may be unsorted
	table.sort(points, sort_func)

        only.log('D', string.format("MilPackage.getAddPoints,key>>%s, data time:[%d, %d], loss time(%d, %d)," .. 
                "coincide time[%d, %d], calcute time[%d, %d]",
                self['recordKey'], dataBeginTime or -1, dataEndTime or -1, lossBeginTime or -1, lossEndTime or -1,
                lossPoints[1]['GPSTime'] or -1, lossPoints[nLosPt]['GPSTime'] or -1, points[1]['GPSTime'] or -1, points[#points]['GPSTime'] or -1 ))

        return points, lossPoints[1]['beginTime'] + 1,lossPoints[nLosPt]['endTime'] - 1
end

-- Calculate the real time data mileage
MilPackage.calcRealMil = function(self)
        local points = self:getRealPoints()

	if not points then
		return 0
	end

        -- update the last point to redis
        writeLastPoint(self['imei'], self['tokenCode'], self['endTime'], self['GPSPoints'][self['endTime']]['speed'], self['GPSPoints'][self['endTime']]['direction'])

        local mileage = self:calcMil(points)

        return mileage
end

-- calculate the additional data mileage
MilPackage.calcAddMil = function(self)
        local points,beginTime,endTime = self:getAddPoints()

        if not points then
                return 0
        end

        --delete this loss area from redis
        delLossPointsOverTime(self['imei'], self['tokenCode'], beginTime, endTime)
        delLossPoints(self['imei'], self['tokenCode'], beginTime, endTime)
        local mileage = self:calcMil(points)

        return mileage
end

MilPackage.startCalc = function(self)
        local mileage = 0

        if self['realFlag'] then
                mileage = self:calcRealMil()
        else
                mileage = self:calcAddMil()
        end

        only.log('D', string.format("key>>%s, normaldata:%s, startGPSTime:%d, endGPSTime:%d, calculated mileage:%f M, calculated accept Count:%d", 
                self['recordKey'], tostring(self['realFlag']), self['startTime'], self['endTime'], mileage, self['acptCount']))

        return mileage
end

