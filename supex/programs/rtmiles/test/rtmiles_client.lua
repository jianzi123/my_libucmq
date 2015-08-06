package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 4140
local serv = "rtmiles"

local function http(cfg, data)
        local tcp = socket.tcp()
        if tcp == nil then
                error('load tcp failed')
                return false
        end
	tcp:settimeout(10000)
        local ret = tcp:connect(cfg["host"], cfg["port"])
	if ret == nil then
		error("connect failed!")
		return false
	end

	tcp:send(data)
	local result = tcp:receive("*a")
	tcp:close()
	return result
end

local function get_data( body )
	return "POST /" .. "publicentry" .. " HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end


local body = {
'{"longitude":[116.3732732,116.3732112,116.3731465,116.373075,116.373006,116.3729347],"latitude":[37.4374908,37.4374942,37.4374966,37.4374977,37.4374975,37.4374957],"IMEI":"103781733780002","model":"V141224_64","collect":true,"speed":[13,16,18,20,20,21], "tokenCode":"Jwlutl3ljS","accountID":"","GPSTime":[1427795419,1427795420,1427795421,1427795422,1427795423,1427795424],"altitude":[14,14,14,13,13,12],"direction":[270,271,271,270,270,269]}',
'{"longitude":[116.3693195,116.3691452,116.3689673,116.3687873,116.3686067,116.3684278],"latitude":[37.4376097,37.4376103,37.4376121,37.437613,37.4376153,37.437619],"IMEI":"103781733780002","model":"V141224_64","collect":true,"speed":[54,55,56,57,57,56], "tokenCode":"Jwlutl3ljS","accountID":"","GPSTime":[1427795456,1427795457,1427795458,1427795459,1427795460,1427795461],"altitude":[11,11,11,11,11,11],"direction":[270,270,270,269,270,270], "extragps":{"longitude":[116.3725235,116.3709857],"latitude":[37.4375223,37.4375602],"IMEI":"103781733780002","model":"V141224_64","collect":true,"speed":[23,40],"tokenCode":"Jwlutl3ljS","accountID":"","GPSTime":[1427795429,1427795445],"altitude":[11,13],"direction":[273,273]}}',
}


for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end

