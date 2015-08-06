--版权声明：暂无
--文件名称：check_gps_parameter.lua
--创建者：滕兴惠
--创建日期：2015/05/20
--文件描述：对数据包中到参数进行检查
--历史记录：无
local only	= require('only')

module('check_gps_parameter',package.seeall)

--名称：ckeck_parameter
--功能：检查数据包的参数是否正确
--参数：数据包
--返回值：检查结果 true-所有参数正确,false-有参数不正确

function check_parameter(args)
--	local imei		= args["IMEI"]
	--local accountID		= args["accountID"]
	local gpstime		= args["GPSTime"]
	local longitude	  	= args["longitude"]
	local latitude     	= args["latitude"]	
	local altitude     	= args["altitude"]
	local speed        	= args["speed"]
	local direction    	= args["direction"]
	
	-->>检查是否为空包	
	if #gpstime == 0 then
		only.log('E',"packet is null")
		return false
	end
	-->>检查imei
--	local imei_status = utils.check_imei(imei)
--	if not imei_status then
--		only.log('E',"imei data error")
--		return false
--	end

	-->>检查 accountID
	--if not accountID  then
	--	only.log('E' , "accountID is nil")
	--	return false
	--else
	--	local id_len = string.len(accountID)
	--	if id_len  ~= 10 and id_len ~= 15 then
	--		return false
	--	end
	--end
	for i=1, #gpstime do
		-->>检查速度	
		if not speed[i] then
			only.log('E',"speed data error")
			return false
		end	
--		-->>检查海拔
--		local val = tonumber(altitude[i])
--		if (not val) or (val < -154.31 or val > 8844.43) then
--			only.log('E',"altitude data error")
--			return false
--		end

		-->>检查方向角
		local val = tonumber(direction[i])
		if (not val) or (val < 0 and val ~= -1) or (val > 360) then
			only.log('E',"direction data error")
			return false
		end

		-->>检查纬度
		local val = tonumber(latitude[i])
		if (not val) or (val > 55.8271 or val < 0.8293) then
			only.log('E',"latitude data error")
			return false
		end

		-->>检查经度
		local val = tonumber(longitude[i])	
		if (not val) or (val > 137.8347 or val < 72.004) then
			only.log('E',"longitude data error")
			return false
		end
		-->>检查GPS时间
		if not gpstime[i] then
			only.log('E',"GPSTime is nil")
			return false
		end
	end
	return true
end

