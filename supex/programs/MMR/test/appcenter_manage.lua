package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local tcp = socket.tcp()
tcp:setoption('keepalive', true)
tcp:settimeout(1, 'b')  -- five second timeout


local ret = tcp:connect("127.0.0.1", 9999)
if ret == nil then
	print("connect failed!")
	return false
end

body = '{"operate":"new_one_app","mode":"alone","tmpname":"_task_dispose_","appname":"a_d_fetch_4_miles_ahead_poi","nickname":"4公里poi播报","args":{},"func":["half_url_mult_idx_send_weibo"]}'
body = '{"operate":"fix_app_cfg","appname":"a_d_fetch_4_miles_ahead_poi","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"half_url_mult_idx_send_weibo":{"app_key":"3406572696","level":70,"idx_key":"4MilesAheadPositionTypeSet","halfURL":"http://127.0.0.1/productList/POIRemind/%s.amr","interval":300}}}}'
body = '{"operate":"ctl_one_app","status":"insmod","mode":"alone","appname":"a_d_fetch_4_miles_ahead_poi"}'


local data = "POST /appcenterMerge.json HTTP/1.0\r\n" ..
"User-Agent: curl/7.33.0\r\n" ..
"Host: 127.0.0.1:9999\r\n" ..
"Connection: close\r\n" ..
"Content-Length:" .. #body .. "\r\n" ..
"Accept: */*\r\n\r\n" ..
body


tcp:send(data)
local result = tcp:receive("*a")
print(result)
tcp:close()
