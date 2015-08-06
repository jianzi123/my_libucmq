package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 8888

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
	return "POST /driviewMerge.json HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end

local history = {
	--'{"operate":"new_one_app","mode":"local","tmpname":"_driving_mileage_","appname":"l_f_driving_mileage","nickname":"连续驾驶里程触发","args":{"collect":["boolean",true],"GPSTime":[],"speed":[],"direction":["function","drive_mileage_point"],"accountID":[]},"func":["app_task_forward"]}',
	--'{"operate":"fix_app_cfg","appname":"l_f_driving_mileage","config":{"bool":{"drive_mileage_point":{"increase":10,"idx_key":"driveOnlineMileagePoint"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"appcenterApply.json?app_name=a_d_driving_mileage&app_mode=alone"}}}}',
	--'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_driving_mileage"}',
	--超速
	'{"operate":"new_one_app","mode":"local","tmpname":"_over_speed_","appname":"l_f_highway_over_speed","nickname":"高速超速触发","args":{"collect":["boolean",true],"speed":["function","check_speed_is_more_than"],"position":["function","judge_position"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_highway_over_speed","config":{"bool":{"check_speed_is_more_than":{"speed":125},"judge_position":{"pos_type":"highway"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"appcenterApply.json?app_name=a_d_highway_over_speed&app_mode=alone"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_highway_over_speed"}',
	--'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_highway_over_speed"}',
	'{"operate":"new_one_app","mode":"local","tmpname":"_over_speed_","appname":"l_f_urban_over_speed","nickname":"城区超速触发","args":{"collect":["boolean",true],"speed":["function","check_speed_is_more_than"],"position":["function","judge_position"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_urban_over_speed","config":{"bool":{"check_speed_is_more_than":{"speed":85},"judge_position":{"pos_type":"urban"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"appcenterApply.json?app_name=a_d_urban_over_speed&app_mode=alone"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_urban_over_speed"}',
	--'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_urban_over_speed"}',
	--'{"operate":"fix_app_cfg","appname":"l_f_fetch_4_miles_ahead_poi","config":{"bool":{"is_4_miles_ahead_have_poi":{"type_delay":300}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"appcenterApply.json?app_name=a_d_fetch_4_miles_ahead_poi&app_mode=alone"}}}}',
	--->>> _drive_pattern_ [auto]
	---->>> mon_driving_mileage [auto]
	'{"operate":"new_one_app","mode":"local","tmpname":"_driving_mileage_","appname":"l_f_mon_driving_mileage","nickname":"月开机行驶里程提醒","args":{"collect":["boolean",true],"GPSTime":[],"speed":[],"direction":["function","mon_drive_mileage_point"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_mon_driving_mileage","config":{"bool":{"mon_drive_mileage_point":{"base_mileage":300,"idx_key":"monDriveOnlineMileagePoint"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_mon_driving_mileage_remind"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_mon_driving_mileage"}',

	----  limit speed remind
	'{"operate":"new_one_app","mode":"local","tmpname":"_road_traffic_","appname":"l_f_limit_speed_remind","nickname":"道路限速提醒","args":{"collect":["boolean",true],"GPSTime":[],"speed":[],"direction":[],"position":["function","is_limit_speed_remind"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_limit_speed_remind","config":{"bool":{"is_limit_speed_remind":{"idx_key":"isLimitSpeedRemind"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_limit_speed_remind"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_limit_speed_remind"}',
	---
	'{"operate":"new_one_app","mode":"local","tmpname":"_road_traffic_","appname":"l_f_urban_mileage_traffic","nickname":"城区里程路况触发","args":{"collect":["boolean",true],"GPSTime":[],"speed":[],"direction":["function","drive_mileage_point"],"position":["function","judge_position"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_urban_mileage_traffic","config":{"bool":{"drive_mileage_point":{"increase":2,"idx_key":"urbanTrafficMileagePoint"},"judge_position":{"pos_type":"urban"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_traffic_remind"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_urban_mileage_traffic"}',
	--- 
	'{"operate":"new_one_app","mode":"local","tmpname":"_road_traffic_","appname":"l_f_road_change_traffic","nickname":"道路切换路况触发","args":{"collect":["boolean",true],"GPSTime":[],"speed":[],"direction":[],"position":["function","is_road_change"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_road_change_traffic","config":{"bool":{},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_traffic_remind"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_road_change_traffic"}',
	-- traffic
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_road_traffic_","remarks":"交通路况","args":["collect","accountID","GPSTime","speed","direction","position"]}',
	'{"operate":"new_one_app","mode":"local","tmpname":"_road_traffic_","appname":"l_f_highway_mileage_traffic","nickname":"高速里程路况触发","args":{"collect":["boolean",true],"GPSTime":[],"speed":[],"direction":["function","drive_mileage_point"],"position":["function","judge_position"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_highway_mileage_traffic","config":{"bool":{"drive_mileage_point":{"increase":30,"idx_key":"highwayTrafficMileagePoint"},"judge_position":{"pos_type":"highway"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_traffic_remind"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_highway_mileage_traffic"}',
	--- drving pattern
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_driving_pattern_","remarks":"驾驶模式","args":["collect","accountID","GPSTime","speed","direction","position"]}',
	'{"operate":"new_one_app","mode":"local","tmpname":"_driving_pattern_","appname":"l_f_driving_pattern","nickname":"驾驶模式判断","args":{"collect":["boolean",true],"GPSTime":[],"speed":[],"direction":[],"accountID":[], "position":["function","driving_pattern_recognition"]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_driving_pattern","config":{"bool":{"driving_pattern_recognition":{}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_driving_pattern_remind"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_driving_pattern"}',
	-- mon drving mileage
	'{"operate":"new_one_tmp","mode":"exact","tmpname":"_power_on_","remarks":"开机启动","args":["powerOn","accountID","tokenCode","model"]}',
	'{"operate":"new_one_app","mode":"exact","tmpname":"_power_on_","appname":"e_f_power_on_mon_driving_mileage","nickname":"开机月驾驶公里数状态提醒","args":{"model":[],"tokenCode":[],"powerOn":["function","power_on_mon_drive_mileage_point"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"e_f_power_on_mon_driving_mileage","config":{"bool":{"power_on_mon_drive_mileage_point":{"base_mileage":300,"idx_key":"monDriveOnlineMileagePoint"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_mon_driving_mileage_remind"}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_f_power_on_mon_driving_mileage"}',	
}


local body = {
	-->[tmp]
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_over_speed_","remarks":"超速驾驶","args":["collect","accountID","speed","position"]}',
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_driving_online_","remarks":"驾驶时长","args":["collect","accountID","GPSTime"]}',
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_driving_mileage_","remarks":"驾驶里程","args":["collect","accountID","GPSTime","speed","direction"]}',
	--'{"operate":"new_one_tmp","mode":"local","tmpname":"_4_miles_ahead_","remarks":"前方4公里","args":["collect","accountID","position"]}',

	--exact
	-- [ctl]	open close insmod rmmod
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_power_on"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_power_on_weibo"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_newbie_guide"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_power_off"}',
	---- e_power_on_status
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_power_on_status"}',	
	'{"operate":"ctl_one_app","status":"insmod","mode":"exact","appname":"e_power_off_status"}',	

	-- local
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_fetch_ambitus_tweet"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_road_traffic"}',
	--'{"operate":"fix_app_cfg","appname":"l_f_road_traffic","config":{"bool":{"check_time_is_between_in":{"time_start":22.5*60*60,"time_end":6*60*60}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_for_ward":{"app_uri":"p2p_traffic_remind"}}}}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_f_road_traffic"}',
	-- 4 miles head
	--'{"operate":"new_one_app","mode":"local","tmpname":"_4_miles_ahead_","appname":"l_f_fetch_4_miles_ahead_poi","nickname":"4公里poi触发","args":{"collect":["boolean",true],"position":["function","is_4_miles_ahead_have_poi"],"accountID":[]},"func":["app_task_forward"]}',
	--'{"operate":"fix_app_cfg","appname":"l_f_fetch_4_miles_ahead_poi","config":{"bool":{"is_4_miles_ahead_have_poi":{"type_delay":300}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_fetch_4_miles_ahead_poi"}}}}',
	--'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_fetch_4_miles_ahead_poi"}',
	-- offsitetri
	'{"operate":"new_one_tmp","mode":"local","tmpname":"_home_offsite_","remarks":"异地","args":["collect","accountID","position"]}',
	'{"operate":"new_one_app","mode":"local","tmpname":"_home_offsite_","appname":"l_f_home_offsite","nickname":"异地触发","args":{"collect":["boolean",true],"position":["function","is_off_site"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_home_offsite","config":{"bool":{},"ways":{"cause":{"trigger_type":"fixed_time","fix_num":1,"delay":180}},"work":{"app_task_forward":{"app_uri":"p2p_offsite_remind"}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_home_offsite"}',
	-- 里程
	'{"operate":"new_one_app","mode":"local","tmpname":"_driving_mileage_","appname":"l_f_continuous_driving_mileage","nickname":"连续驾驶里程触发","args":{"collect":["boolean",true],"GPSTime":[],"speed":[],"direction":["function","is_continuous_driving_mileage_point"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_continuous_driving_mileage","config":{"bool":{"is_continuous_driving_mileage_point":{"increase":10,"idx_key":"continuousDrivingMileage"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_continuous_driving_mileage_remind"}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_continuous_driving_mileage"}',
	-- 时长
	'{"operate":"new_one_app","mode":"local","tmpname":"_driving_online_","appname":"l_f_fatigue_driving","nickname":"疲劳驾驶触发","args":{"collect":["boolean",true],"GPSTime":["function","drive_online_point"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_fatigue_driving","config":{"bool":{"drive_online_point":{"increase":3600,"idx_key":"driveOnlineHoursPoint"}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"appcenterApply.json?app_name=a_d_fatigue_driving&app_mode=alone"}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_fatigue_driving"}',
	'{"operate":"ctl_one_app","status":"rmmod","mode":"local","appname":"l_collect"}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_collect_gps"}',
	-- l_f_weather_forcast
	--'{"operate":"new_one_app","mode":"local","tmpname":"_4_miles_ahead_","appname":"l_f_weather_forcast","nickname":"天氣預報","args":{"collect":["boolean",true],"position":["function","is_weather_forcast"],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_weather_forcast","config":{"bool":{"is_weather_forcast":{"type_delay":300}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_weather_forcast"}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_weather_forcast"}',

	-->>超速
	'{"operate":"new_one_app","mode":"local","tmpname":"_over_speed_","appname":"l_f_over_speed","nickname":"超速提醒","args":{"collect":["boolean",true],"speed":["function","is_over_speed"],"position":[],"accountID":[]},"func":["app_task_forward"]}',
	'{"operate":"fix_app_cfg","appname":"l_f_over_speed","config":{"bool":{"is_over_speed":{"speed":125}},"ways":{"cause":{"trigger_type":"every_time","fix_num":1,"delay":0}},"work":{"app_task_forward":{"app_uri":"p2p_over_speed"}}}}',
	'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_over_speed"}',
	--->>> test [auto]
	--'{"operate":"ctl_one_app","status":"insmod","mode":"local","appname":"l_f_test"}',
}

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
