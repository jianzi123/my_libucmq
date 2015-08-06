package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 7777
local serv = "ptop"

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
	--return "POST /w_xxx HTTP/1.0\r\n" ..
	--return "POST /p2p_traffic_remind HTTP/1.0\r\n" ..
	--return "POST /p2p_offsite_remind HTTP/1.0\r\n" ..
	--return "POST /p2p_get_txt2voice HTTP/1.0\r\n" ..
	return "POST /p2p_fetch_4_miles_ahead_poi HTTP/1.0\r\n" ..
	--return "POST /p2p_driving_pattern_remind HTTP/1.0\r\n" ..
	--return "POST /p2p_weather_forcast HTTP/1.0\r\n" ..
	--return "POST /p2p_continuous_driving_mileage_remind HTTP/1.0\r\n" ..
	--return "POST /p2p_over_speed HTTP/1.0\r\n" ..
	--return "POST /p2p_limit_speed_remind HTTP/1.0\r\n" ..
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
        --北京东经：116°23′17〃，北纬：39°54′27
        --北京东经：116.230017〃，北纬：39.542700
        --'{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221], "longitude":[116.230017], "latitude":[39.542700], "speed":[150], "direction":[100]}',
        --'{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029], "speed":[150,120,110], "direction":[23,50,100]}',
        --'{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.50149876,121.758718,121.858718], "latitude":[31.23700579,31.322029,31.342029], "speed":[150,120,110], "direction":[23,50,100]}',
        --'{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.3977898,121.3977898,121.3977898], "latitude":[31.2202088,31.2202088,31.2202088], "speed":[23,23,23], "direction":[150,150,150], "altitude":[23,2,3]}',
        --'{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.3636108,121.3636108,121.3636108], "latitude":[31.2245105,31.2245105,31.2245105], "speed":[23,23,23], "direction":[123,123,123], "altitude":[23,2,3]}',
        '{"collect":true, "accountID":"a5ytlr93ap", "GPSTime":[1231221,1231221,1231221], "longitude":[121.3636108,121.3636108,121.3636108], "latitude":[31.2245105,31.2245105,31.2245105], "speed":[23,23,23], "direction":[123,123,123], "altitude":[23,2,3]}',
}

for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end
