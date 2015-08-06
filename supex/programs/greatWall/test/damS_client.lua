package.cpath = "../../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 4210
local serv = "damS"

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
	return "POST /" .. serv .. "Apply.json HTTP/1.0\r\n" ..
	--return "POST /w_xxx HTTP/1.0\r\n" ..
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
        '{"collect":true, "accountID":"850212090004492", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029], "speed":[150,120,110], "direction":[23,50,100]}',
}

for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end

-------|-----------------|---------------------
--方法-|-link对应的server| 具体的body内容 -----
--lpushx 0|1|2 "POST /driviewApply.json HTTP/1.0\r\nHost: 127.0.0.1:8888\r\nConnection: close\r\n\r\n"

--printf "GET / HTTP/1.0\r\nHost: www.baidu.com:80\r\nConnection: close\r\n\r\n" | nc www.baidu.com 80
--printf "*3\r\n\$6\r\nLPUSHX\r\n\$1\r\n0\r\n\$77\r\nPOST /driviewApply.json HTTP/1.0\r\nHost: 127.0.0.1:8888\r\nConnection: close\r\n\r\n\r\n" | nc 192.168.1.15 5000
