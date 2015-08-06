package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 2222
local serv = "drimode"

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
'{"longitude":[116.3732732],"latitude":[37.4374908],"IMEI":"103781733780002","model":"V141224_64","collect":true,"speed":[13], "tokenCode":"Jwlutl3ljS","accountID":"","GPSTime":[1430958000],"altitude":[14],"direction":[270]}',
'{"longitude":[116.3693195],"latitude":[37.4376097],"IMEI":"103781733780002","model":"V141224_64","collect":true,"speed":[55], "tokenCode":"Jwlutl3ljS","accountID":"","GPSTime":[1427795456],"altitude":[11],"direction":[270], "extragps":{"longitude":[116.3725585],"latitude":[37.4375223],"IMEI":"103781733780002","model":"V141224_64","collect":true,"speed":[23],"tokenCode":"Jwlutl3ljS","accountID":"","GPSTime":[1430959200],"altitude":[11],"direction":[273]}}',
'{"longitude":[116.3732732,118.757547,118.7575118,118.7575223,118.7574588],"latitude":[37.4374908,32.1077803,32.10771,32.1075565,32.1074165],"IMEI":"430216181295504","model":"SG900","collect":true,"speed":[32,27,27,22,18],"tokenCode":"BMspYupyYf","accountID":"llUzKkrtAp","GPSTime":[1430958000,1430958320,1430958321,1430958000,1430958600],"altitude":[53,52,54,59,61],"direction":[30,24,20,17,17]}',
'{"longitude":[118.7597457,118.7597457,118.7597457,118.7597457,118.7597457],"latitude":[32.1080148,32.1080148,32.1080148,32.1080148,32.1080148],"IMEI":"430216181295504","model":"SG900","collect":true,"speed":[0,0,0,0,0],"tokenCode":"BMspYupyYf","accountID":"llUzKkrtAp","GPSTime":[1430924242,1430924241,1430924240,1430924239,1430924238],"altitude":[17,17,17,17,17],"direction":[-1,-1,-1,-1,-1]}',

}


for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end

