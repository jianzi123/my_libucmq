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


local body = {
	--string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":"kkoo","FUNC":"test","TIME":%d,"LIVE":1}',
	--(os.time()+ 8*60*60) %(60 *60 *24)),
	--string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":{"accountID":"kxl1QuHKCD", "tokenCode":"czlhlRzmaA"},"FUNC":"entry","TIME":%d,"LIVE":1}',
	--(os.time()+ 8*60*60) %(60 *60 *24)),
	--string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":{"accountID":"kxl1QuHKCD"},"FUNC":"entry","TIME":%d,"LIVE":1}',
	--(os.time()+ 8*60*60) %(60 *60 *24)),
	--string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":{"tokenCode":"lN6GQBBCo5","imei":"800981013883924","startTime":"1404367200","endTime":"1404370800"},"FUNC":"entry","TIME":%d,"LIVE":1}',
	--(os.time()+ 8*60*60 ) %(60 *60 *24)),
	--string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":{"tokenCode":"2iifiYhlKL","imei":"392227436589951","startTime":"1413406403","endTime":"1413447727"},"FUNC":"entry","TIME":%d,"LIVE":1}',
	--(os.time()+ 8*60*60 ) %(60 *60 *24)),
}


for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end

body = {
	--string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":"kkoo","FUNC":"test","TIME":%d,"LIVE":1}',
	--(os.time()+ 8*60*60) %(60 *60 *24)),
	--string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":{"accountID":"kxl1QuHKCD", "tokenCode":"czlhlRzmaA"},"FUNC":"entry","TIME":%d,"LIVE":1}',
	--(os.time()+ 8*60*60) %(60 *60 *24)),
	--string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":{"accountID":"kxl1QuHKCD"},"FUNC":"entry","TIME":%d,"LIVE":1}',
	--(os.time()+ 8*60*60) %(60 *60 *24)),
	--string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":{"tokenCode":"lN6GQBBCo5","imei":"800981013883924","startTime":"1404367200","endTime":"1404370800"},"FUNC":"entry","TIME":%d,"LIVE":1}',
	--(os.time()+ 8*60*60 + 180) %(60 *60 *24)),
	string.format('{"APPLY":"gopath","MODE":"push","EXEC":"make","ARGS":{"tokenCode":"2iifiYhlKL","imei":"392227436589951","startTime":"1413406403","endTime":"1413447727"},"FUNC":"entry","TIME":%d,"LIVE":1}',
	(os.time()+ 8*60*60 + 60 ) %(60 *60 *24)),
}

function handle()
	--for i = 0, 10000 do
	for i = 0, 10 do
		for idx,val in pairs(body) do
			--print(idx,val)
			local data = get_data(val)
			print(data)
			local info = http({host = host, port = port}, data)
			print(info)
		end
	end
end
handle()
