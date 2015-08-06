package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 9999

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
	return "POST /appcenterMerge.json HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end


local body = {
	-->[tmp]
	'{"operate":"new_one_tmp","mode":"alone","tmpname":"_task_dispose_","remarks":"任务执行","args":[]}',

	-->[new]
	-->>位置
	'{"operate":"new_one_app","mode":"alone","tmpname":"_task_dispose_","appname":"a_d_fetch_4_miles_ahead_poi","nickname":"4公里poi播报","args":{},"func":["half_url_mult_idx_send_weibo"]}',
	-->>超速
	'{"operate":"new_one_app","mode":"alone","tmpname":"_task_dispose_","appname":"a_d_highway_over_speed","nickname":"高速超速提醒","args":{},"func":["full_url_send_weibo"]}',
	'{"operate":"new_one_app","mode":"alone","tmpname":"_task_dispose_","appname":"a_d_urban_over_speed","nickname":"城区超速提醒","args":{},"func":["full_url_send_weibo"]}',
	-->>时长
	'{"operate":"new_one_app","mode":"alone","tmpname":"_task_dispose_","appname":"a_d_fatigue_driving","nickname":"疲劳驾驶提醒","args":{},"func":["half_url_incr_idx_send_weibo"]}',
	-->>里程
	'{"operate":"new_one_app","mode":"alone","tmpname":"_task_dispose_","appname":"a_d_driving_mileage","nickname":"连续驾驶里程提醒","args":{},"func":["half_url_incr_idx_send_weibo"]}',

	-->[fix]
	'{"operate":"fix_app_cfg","appname":"a_d_fetch_4_miles_ahead_poi","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"half_url_mult_idx_send_weibo":{"app_key":"2491067261","secret":"52E8DCDEB8DBBAD220652851AE34339B008F5B48","level":70,"idx_key":"4MilesAheadPositionTypeSet","carry_key":"4MilesAheadPositionCarry","halfURL":"http://127.0.0.1/productList/POIRemind/%s.amr","interval":30}}}}',

	'{"operate":"fix_app_cfg","appname":"a_d_highway_over_speed","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"full_url_send_weibo":{"app_key":"3406572696","secret":"04A069DD5AAFE20712CFE846650E02D239C1D4C1","level":25,"fullURL":"http://127.0.0.1/productList/overSpeed/urbanArea/20001.amr","interval":300}}}}',

	'{"operate":"fix_app_cfg","appname":"a_d_urban_over_speed","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"full_url_send_weibo":{"app_key":"3406572696","secret":"04A069DD5AAFE20712CFE846650E02D239C1D4C1","level":25,"fullURL":"http://127.0.0.1/productList/overSpeed/highway/20001.amr","interval":300}}}}',

	'{"operate":"fix_app_cfg","appname":"a_d_fatigue_driving","config":{"bool":{},"ways":{"cause":{"trigger_type":"fixed_time","fix_num":1,"delay":300}},"work":{"half_url_incr_idx_send_weibo":{"level":80,"idx_key":"driveOnlineHoursPoint","halfURL":"http://127.0.0.1/productList/fatigueDriving/%s.amr","app_key":"179429838","secret":"D2715E17E86D292BB2B9EA494E532F85F22EE9DB","idx_max":28,"idx_base":"30000","interval":300}}}}',

	'{"operate":"fix_app_cfg","appname":"a_d_driving_mileage","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"half_url_incr_idx_send_weibo":{"level":80,"idx_key":"driveOnlineMileagePoint","halfURL":"http://127.0.0.1/productList/drivingMileage/%s.amr","app_key":"1106086205","secret":"C6BF9297AAE1A12E267FBE4DED1C99FB54132607","idx_base":"40000","idx_max":51,"interval":300}}}}',

	-->[ctl]	open close insmod rmmod
	-->>exact
	-->>local
	-->>alone
	'{"operate":"ctl_one_app","status":"insmod","mode":"alone","appname":"a_d_fetch_4_miles_ahead_poi"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"alone","appname":"a_d_highway_over_speed"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"alone","appname":"a_d_urban_over_speed"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"alone","appname":"a_d_fatigue_driving"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"alone","appname":"a_d_driving_mileage"}',
	-- a_app_solarcalendar
	'{"operate":"ctl_one_app","status":"insmod","mode":"alone","appname":"a_app_solarcalendar"}',

	'{"operate":"ctl_one_app","status":"insmod","mode":"alone","appname":"a_app_report_user_online"}',
	--'{"operate":"ctl_one_app","status":"insmod","mode":"alone","appname":"a_app_report_user_offline"}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"alone","appname":"a_app_report_user_offline"}',

}

for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end
