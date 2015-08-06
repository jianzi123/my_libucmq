--版权声明：无
--文件名称：match_imei_record.lua
--创建者：胡文灿
--创建日期：2015.6.3
--文件描述：本文件主要处理上传的GPS数据，找出同一时间在相同1/100格网内的所有imei，以"timer:loaction:gridkey"为key,imei为值存入redis数据库中
--历史记录：无
local only = require('only')
local cjson = require('cjson')
local redis_api = require('redis_pool_api')
local pool = require('pool')
local check_gps_parameter = require('check_gps_parameter')
local LOCATIONKEY = "locationkey"

module('match_imei_record',package.seeall)

local function process_grid(body,imei)
	--取出GPS包中的TIME数据利用最新的时间点来创建时间戳
	local timestamp = ((math.floor(body['GPSTime'][1]/600))*600)
	for i=1,#body['GPSTime'] do
		if (timestamp==(body['GPSTime'][i])) then
			local lon = body['longitude'][i]
			local lat = body['latitude'][i]
			local gridkey = string.format("%d:%d%d:%s",body['GPSTime'][i],math.floor(lon*100),math.floor(lat*100),"gridkey")
			local ok,inert = redis_api.cmd('equipmentInfo','','SADD',gridkey,imei)
			local ok,nameset = redis_api.cmd('equipmentInfo','','RPUSH',LOCATIONKEY,gridkey)
			if not ok then
					only.log("D","insert to redis failed")
				end
			break
		end
	end
end

function handle()
	local body = pool["OUR_BODY_TABLE"]
	local imei = body['IMEI']
	local ok = check_gps_parameter.check_parameter(body)
	if not ok then
		only.log("E","error body data")
		return false
	end
	process_grid(body,imei)
	--若有补传数据则计算补传数据
		if (body['extragps']) then
			process_grid(body['extragps'],imei)
		end
end
