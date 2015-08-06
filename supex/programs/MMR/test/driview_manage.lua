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


body = '{"operate":"new_one_tmp","mode":"local","tmpname":"_4_miles_ahead_","remarks":"前方4公里","args":["collect","accountID","position"]}'
body = '{"operate":"new_one_app","mode":"local","tmpname":"_4_miles_ahead_","appname":"l_f_fetch_4_miles_ahead_poi","nickname":"4公里poi触发","args":{"collect":["boolean",true],"position":["function","is_4_miles_ahead_have_poi"],"accountID":[]},"func":["app_task_forward"]}'
body = '{"operate":"fix_app_cfg","appname":"l_f_fetch_4_miles_ahead_poi","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"a_d_fetch_4_miles_ahead_poi"}}}}'
body = '{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_fetch_4_miles_ahead_poi"}'

local data = "POST /driviewMerge.json HTTP/1.0\r\n" ..
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
