package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 4120

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
	return "POST /mddsMerge.json HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end

local body = {
	-->[tmp]
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_4_miles_ahead_","remarks":"前方4公里","args":["collect","accountID","position"]}',
	-- local
	-- 4 miles head
	'{"operate":"new_one_app","mode":"local","tmpname":"_4_miles_ahead_","appname":"l_f_fetch_4_miles_ahead_poi","nickname":"4公里poi触发","args":{"collect":["boolean",true],"position":["function","is_4_miles_ahead_have_poi"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_fetch_4_miles_ahead_poi","config":{"bool":{"is_4_miles_ahead_have_poi":{"type_delay":300}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_fetch_4_miles_ahead_poi"}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_fetch_4_miles_ahead_poi"}',
}

--a sample for aotucode
--local body = {
--->>> test [auto]
--	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_test"}',
--}

for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end
