package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 4444
local serv = "crazypoint"

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

local data = "GET /w_xxx HTTP/1.0\r\n" ..
"User-Agent: curl/7.33.0\r\n" ..
string.format("Host: %s:%d\r\n", host, port) ..
"Connection: close\r\n" ..
"Accept: */*\r\n\r\n"

print(data)
local info = http({host = host, port = port}, data)
print(info)
