package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')
local	MAX = 1
for i=1, MAX do

	local tcp = socket.tcp()
	tcp:setoption('keepalive', true)
	tcp:settimeout(1, 'b')  -- five second timeout

	local ret = tcp:connect("127.0.0.1", 8888)
	if ret == nil then
		return false
	end

	body = '{"powerOn":true,	"accountID":"850212090004492",	"tokenCode":"1adada912939",	"model":"SG900"}'
	body = '{"powerOff":true,	"accountID":"850212090004492",	"tokenCode":"1adada912939",	"model":"SG900"}'
	body = '{"collect":true,	"accountID":"850212090004492",	"tokenCode":"1adada912939"}'
	body = '{"collect":true,	"accountID":"850212090004492",	"GPSTime":["1231221"]}'
	body = '{"collect":true,	"accountID":"850212090004492",	"GPSTime":["1231222","1231221"], "direction":[14,12], "speed":[52300,30900]}'
	body = '{"collect":true,	"accountID":"850212090004492",	"longitude":["121.358717"], "latitude":["31.222029"], "speed":[150]}'
	body = '{"collect":true, "accountID":"850212090004492", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029], "speed":[150,120,110], "direction":[23,50,100]}'
	body = '{"powerOn":true,	"accountID":"zdfeqE74Vi",	"tokenCode":"1adada912939",	"model":"SG900"}'

	body = '{"collect":true,	"accountID":"zdfeqE74Vi",	"GPSTime":["1231222","1231221"], "direction":[14,12], "speed":[52300,30900]}'
	body = '{"powerOn":true,	"accountID":"zdfeqE74Vi",	"tokenCode":"1adada912939",	"model":"SG900"}'
	body = '{"collect":true,	"accountID":"zdfeqE74Vi",	"GPSTime":["1231222","1231221"], "direction":[14,12], "speed":[52300,30900]}'
	--body = '{"collect":true,	"accountID":"SelJNQsF7B",	"GPSTime":["1231222","1231221"], "direction":[14,12], "speed":[52300,30900]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029], "speed":[150,120,110], "direction":[23,50,100]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029], "speed":[1,20,300], "direction":[23,50,100]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718,121.858718,121.858718], "latitude":[31.222029,31.322029,31.342029,31.342029,31.342029], "speed":[100,90,100,110,120], "direction":[23,50,100,23,23]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.358717,121.358717], "latitude":[31.222029,31.222029,31.222029], "altitude":[31,31,31],"speed":[10,20,300], "direction":[23,50,100]}'
	--body = '{"collect":true, "accountID":"zdfeqE74Vi", "longitude":[121.358717,121.758718,121.858718,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029,31.322029,31.342029], "direction":[23,50,100,50,100],"speed":[80,80,80,80,80],"altitude":[31,31,31,31,31],"GPSTime":[1231221,1231221,1231221,1231221,1231221]}'
	body = '{"collect":true, "accountID":"a5ytlr93ap", ' .. 
	'"longitude":[121.3636108,121.3636108,121.3636108,121.3636108,121.3636108],'..
	' "latitude":[31.2245105,31.2245105,31.2245105,31.2245105,31.2245105],'.. 
	' "direction":[123,123,123,123,123],'..
	'"speed":[180,60,50,40,30],'..
	'"altitude":[31,31,31,31,31],'..
	'"GPSTime":[1231221,1231221,1231221,1231221,1231221]}'
	body = '{"collect":true, "accountID":"zdfeqE74Vi", "longitude":[121.3977898,121.3977898,121.3977898,121.3977898,121.3977898], "latitude":[31.2202088,31.2202088,31.2202088,31.2202088,31.2202088], "direction":[150,150,150,150,150],"speed":[80,80,80,80,80],"altitude":[31,31,31,31,31],"GPSTime":[1231221,1231221,1231221,1231221,1231221]}'
	--body = '{"powerOn":true,	"accountID":"zdfeqE74Vi",	"tokenCode":"1adada912939",	"model":"SG900"}'
	local data = "POST /publicentry HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	"Host: 127.0.0.1:8888\r\n" ..
	"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body


	tcp:send(data)
	local result = tcp:receive("*a")
	print(result)
	tcp:close()
end
--os.execute("sleep 10")

