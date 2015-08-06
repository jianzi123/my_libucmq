package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 3333
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

local function get_data( body )
	return "POST /crazypointApply.json HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end

body = {
	string.format('{"APPLY":"w_xxx","MODE":"insert","EXEC":"make","ARGS":{"tokenCode":"HnHpllpimZ","imei":"860122717752589","startTime":"1413784067","endTime":"1414025017"},"TIME":%d,"LIVE":1}',
	(os.time()+ 8*60*60 + 30) %(60 *60 *24)),
}



for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	--print(data)
	local info = http({host = host, port = port}, data)
	--print(info)
    --print("jizhong")
end

