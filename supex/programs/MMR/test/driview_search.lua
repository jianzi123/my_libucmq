package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local tcp = socket.tcp()
tcp:setoption('keepalive', true)
tcp:settimeout(1, 'b')  -- five second timeout


local ret = tcp:connect("127.0.0.1", 8888)
if ret == nil then
	print("connect failed!")
	return false
end

body = '{"operate":"get_all_app","mode":"all"}'
--body = '{"operate":"get_all_was"}'
--body = '{"operate":"get_tmp_app","mode":"local","tmpname":"fatigue_driving_remind"}'
--body = '{"operate":"get_app_cfg","appname":"l_urban_over_speed_remind"}'
--body = '{"operate":"get_app_exp","appname":"l_drive_status"}'
--body = '{"operate":"get_all_tmp","mode":"whole"}'
--body = '{"operate":"get_tmp_arg","mode":"whole","tmpname":"driview_status"}'
--body = '{"operate":"get_all_job","mode":"alone"}'
--body = '{"operate":"get_all_arg"}'

local data = "POST /driviewFetch.json HTTP/1.0\r\n" ..
"User-Agent: curl/7.33.0\r\n" ..
"Host: 127.0.0.1:8888\r\n" ..
"Connection: close\r\n" ..
"Content-Length:" .. #body .. "\r\n" ..
"Accept: */*\r\n\r\n" ..
body

tcp:send(data)
local result = tcp:receive("*a")
print(result)
tcp:close()
