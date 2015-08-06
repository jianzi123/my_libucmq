package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')
local file_gps_packet = require('file_gps_packet')

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
	return "POST /publicentry HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end


local body = file_gps_packet.file_gps()
for idx,val in pairs(body) do
	--print(idx,val)
	local t0 = os.clock()
		while os.clock() - t0 <= 0.2 do
		end
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end
