-- Auther:GengXuanxuan
-- Date:2014-04-13
-- Feature:#2267
--
-- Redis record class of Mileage
local mil_defs = require('mil_defs')

module("mil_record", package.seeall)




-- Mileage record that map hash objects of redis
MilRecord = {
        recordKey = "",         -- Redis hash key -->  mileage:imei:tokenCode
        realFlag = true,        -- true: Real time GPS Data; false: additional data
        businessID = 0,         -- Default is 0
        bonusType = 1,       -- Default is 1

        -- the fallow fields need calculation every times 
        sumMileageMts = 0,      -- format %d, Meters
        actualMileageMts = 0,   -- format %d, Meters
        GPSAcpCount = 0,        -- Accepted GPS points of real time

	lowSpeedTime = 0,
	stopTime = 0,
	maxSpeed = 0,
	speedSum = 0,
	acptCount = 0,
	actualSpeedSum = 0,
	actualGPSSum = 0,
}

-- Return new object to MilRecord
MilRecord.new = function(self)
        local attr = {}

        setmetatable(attr, self)
        self.__index = self

        return attr
end

-- Init MilRecord object by request body
MilRecord.initFromData = function(self, milpkg)
	self['imei']         = milpkg['imei']
	self['tokenCode']    = milpkg['tokenCode']
	self['accountID']    = milpkg['accountID']
	self['createTime']   = milpkg['createTime']
        self['realFlag']     = milpkg['realFlag']
        self['recordKey']    = milpkg['recordKey']

        if not milpkg['logined'] then
                self['startTime'] = milpkg['startTime']
                self['startLongitude'] = milpkg['startLongitude']
                self['startLatitude'] = milpkg['startLatitude']
                self['startDirection'] = milpkg['startDirection']
        end

        self['endTime'] = milpkg['endTime']
        self['endLongitude'] = milpkg['endLongitude'] 
        self['endLatitude'] = milpkg['endLatitude']
        self['endDirection'] = milpkg['endDirection']

        --travelID
        --accountID
        --businessID
        --bonusType
        --startProvinceName
        --startProvinceCode
        --startCityName
        --startCityCode
        --startCountyName
        --startCountyCode
        --startRoadName
        --startPOIName

        return true
end

MilRecord.initFromRedis = function(self,recordKey)
        self['recordKey'] = recordKey
        local   ok,ret = redis_pool_api.cmd(mil_defs['STATIC']['REDIS_NAME'], self['accountID'] or '', "HMGET", self['recordKey'],
                'imei',
                'tokenCode',
                'createTime',
                'startLongitude',
                'startLatitude',
                'startDirection',
                'startTime',
                'endLongitude',
                'endLatitude',
                'endDirection',
                'endTime',
                'GPSLossRate',
                'sumMileageMts',
                'actualMileageMts',
                'GPSAcpCount',
		'lowSpeedTime',
		'stopTime',
		'maxSpeed',
		'actualSpeedSum',
		'actualGPSSum')

        if (not ok) or (not ret) then
                only.log('E', string.format("get record from redis error, record key>>%s",
                        self['recordKey']))

                return nil
        end
        
        self['imei'] = ret[1]
        self['tokenCode'] = ret[2]
        self['createTime'] = tonumber(ret[3])
        self['startLongitude'] = tonumber(ret[4])
        self['startLatitude'] = tonumber(ret[5])
        self['startDirection'] = tonumber(ret[6])
        self['startTime'] = tonumber(ret[7])
        self['endLongitude'] = tonumber(ret[8])
        self['endLatitude'] = tonumber(ret[9])
        self['endDirection'] = tonumber(ret[10])
        self['endTime'] = tonumber(ret[11])
        self['GPSLossRate'] = tonumber(ret[12])
        self['sumMileageMts'] = tonumber(ret[13])
        self['actualMileageMts'] = tonumber(ret[14])
        self['GPSAcpCount'] = tonumber(ret[15])
	self['lowSpeedTime'] = tonumber(ret[16])
	self['stopTime'] = tonumber(ret[17])
	self['maxSpeed'] = tonumber(ret[18])
	self['actualSpeedSum'] = tonumber(ret[19])
	self['actualGPSSum'] = tonumber(ret[20])

        return true
end

-- set new mileage and accpted count 
MilRecord.setNewData = function(self, mileage, acptCount, normal, lowSpeedTime, stopTime, maxSpeed, speedSum)
	if self['realFlag'] then
		--有效里程
        	self['sumMileageMts'] = self['sumMileageMts'] + mileage
		--GPS丢失率
        	self['GPSAcpCount'] = self['GPSAcpCount'] + acptCount
        	local lossRate = ((self['endTime'] - self['startTime'] + 1 - self['GPSAcpCount']) / (self['endTime'] - self['startTime'] + 1)) * 100
        	self['GPSLossRate'] = lossRate - lossRate%1
	end

	--实际里程
        self['actualMileageMts'] = self['actualMileageMts'] + mileage
	--异常关机标志
	if normal then
		self['normal'] = normal
	end
	--低速行驶时间
	self['lowSpeedTime'] = self['lowSpeedTime'] + lowSpeedTime
	--停车时间
	self['stopTime'] = self['stopTime'] + stopTime
	--行车时间
	self['driveTime'] = self['endTime'] - self['startTime']
	--最大速度
	if self['maxSpeed'] < maxSpeed then
		self['maxSpeed'] = maxSpeed
	end
	--平均速度
	self['actualSpeedSum'] = self['actualSpeedSum'] + speedSum
	self['actualGPSSum'] = self['actualGPSSum'] + acptCount
	if self['actualGPSSum'] == 0 then
		self['avgSpeed'] = 0
	else
		local avgSpeed = self['actualSpeedSum'] / self['actualGPSSum']
		self['avgSpeed'] = avgSpeed - avgSpeed % 1
	end
end

-- Update Record to Redis
MilRecord.update = function(self)
        local ok, ret = redis_pool_api.cmd(mil_defs['STATIC']['REDIS_NAME'],self['accountID'] or "", "HMSET", self['recordKey'],
                'startTime', self['startTime'],
                'startLongitude', self['startLongitude'],
                'startLatitude', self['startLatitude'],
                'startDirection', self['startDirection'],
                'endTime', self['endTime'],
                'endLongitude', self['endLongitude'],
                'endLatitude', self['endLatitude'],
                'endDirection', self['endDirection'],
                'sumMileageMts', self['sumMileageMts'],
                'actualMileageMts', self['actualMileageMts'],
                'GPSAcpCount', self['GPSAcpCount'],
                'GPSLossRate', self['GPSLossRate'],
		'normal', self['normal'],
		'lowSpeedTime', self['lowSpeedTime'],
		'stopTime', self['stopTime'],
		'driveTime', self['driveTime'],
		'maxSpeed', self['maxSpeed'],
		'avgSpeed', self['avgSpeed'],
		'actualSpeedSum', self['actualSpeedSum'],
		'actualGPSSum', self['actualGPSSum'])

        if (not ok) or (not ret) then
                only.log('E', string.format("update mileage record error,key>>%s",
                        self['recordKey']))
        end

        -- write to last gps time to, wait for data curing
        ok , ret = redis_pool_api.cmd(mil_defs['STATIC']['REDIS_NAME'], self['accountID'] or '', "ZADD", mil_defs['STATIC']['LASTED_MILE_ZSET'], self['endTime'], self['recordKey']) 
        if not ok then 
                only.log('E', string.format("Write %s to mileagerecordset error", recordKey))
                return
        end
end

MilRecord.write = function(self)
        ok , ret = redis_pool_api.cmd(mil_defs['STATIC']['REDIS_NAME'], self['accountID'] or "" ,"HMSET", self['recordKey'], 
                        'travelID', self['travelID'],
                        'accountID', self['accountID'],
                        'businessID', self['businessID'],
                        'bonusType', self['bonusType'],
                        'imei', self['imei'],
                        'tokenCode', self['tokenCode'],
                        'createTime', self['createTime'],
                        'startProvinceName', self['startProvinceName'] or "",
                        'startProvinceCode', self['startProvinceCode'] or "",
                        'startCityName', self['startCityName'] or "",
                        'startCityCode', self['startCityCode'] or "",
                        'startCountyName', self['startCountyName'] or "",
                        'startCountyCode', self['startCountyCode'] or "",
                        'startLongitude', self['startLongitude'],
                        'startLatitude', self['startLatitude'],
                        'startDirection', self['startDirection'],
                        'startTime', self['startTime'],
                        'startRoadName', self['startRoadName'] or "",
                        'startPOIName', self['startPOIName'] or "",
                        'endProvinceName', self['endProvinceName'] or "",
                        'endProvinceCode', self['endProvinceCode'] or "",
                        'endCityName', self['endCityName'] or "",
                        'endCityCode', self['endCityCode'] or "",
                        'endCountyName', self['endCountyName'] or "",
                        'endCountyCode', self['endCountyCode'] or "",
                        'endLongitude', self['endLongitude'],
                        'endLatitude', self['endLatitude'],
                        'endDirection', self['endDirection'],
                        'endTime', self['endTime'],
                        'endRoadName', self['endRoadName'] or "",
                        'endPOIName', self['endPOIName'] or "",
                        'GPSLossRate', self['GPSLossRate'],
                        'sumMileageMts', self['sumMileageMts'],
                        'actualMileageMts', self['actualMileageMts'],
                        'GPSAcpCount', self['GPSAcpCount'],
			'normal', self['normal'] or 1,
			'lowSpeedTime', self['lowSpeedTime'] or 0,
			'stopTime', self['stopTime'] or 0,
			'driveTime', self['driveTime'] or 0,
			'maxSpeed', self['maxSpeed'] or 0,
			'avgSpeed', self['avgSpeed'] or 0,
			'actualSpeedSum', self['actualSpeedSum'] or 0,
			'actualGPSSum', self['actualGPSSum'] or 0)

        if (not ok) or (not ret) then
                only.log('E', string.format("write mileage record error, record key>>%s", self['recordKey']))
                return
        end

        -- write to last gps time to, wait for data curing
        ok , ret = redis_pool_api.cmd(mil_defs['STATIC']['REDIS_NAME'], self['accountID'] or '', "ZADD", mil_defs['STATIC']['LASTED_MILE_ZSET'], self['endTime'], self['recordKey']) 
        if not ok then 
                only.log('E', string.format("Write %s to LASTED_MILE_ZSET error", recordKey))
                return
        end

        -- write mileage count
        ok , ret = redis_pool_api.cmd(mil_defs['STATIC']['REDIS_NAME'], self['accountID'] or '', "INCR", 'mileageCount') 
        if not ok then 
                only.log('E', "incr mileageCount error")
                return
        end
end

MilRecord.updNewStart = function(self, startTime, startLongitude, startLatitude, startDirection)
        if self.realFlag then
                return
        end

        if not self['startTime'] or startTime < self['startTime'] then
                self['startTime'] = startTime
                self['startLongitude'] = startLongitude 
                self['startLatitude'] = startLatitude
                self['startDirection'] = startDirection
        end
end

MilRecord.updNewEnd = function(self, endTime, endLongitude, endLatitude, endDirection)
        if endTime > self['endTime'] then
                self['endTime'] = endTime
                self['endLongitude'] = endLongitude 
                self['endLatitude'] = endLatitude
                self['endDirection'] = endDirection
        end
end

