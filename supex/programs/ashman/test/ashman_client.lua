package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')

local host = "127.0.0.1"
local port = 4080
local serv = "ashman"

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
	return "POST /ashman_test HTTP/1.0\r\n" ..
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
        --'{"collect":true, "accountID":"850212090004492", "GPSTime":[1231221,1231221,1231221], "longitude":[121.358717,121.758718,121.858718], "latitude":[31.222029,31.322029,31.342029], "speed":[150,120,110], "direction":[23,50,100]}',
        --'{"tokenCode":"HnHpllpimZ","imei":"860122717752589","startTime":"1413784067","endTime":"1414025017"}'
        --'{"imei":"683196015727290",   "tokenCode":"8wvmz2zl6D",       "startTime":"1414705340",       "endTime":"1414865397"}',  
        --'{"imei":"592014033207700",   "tokenCode":"ulQ6Rslzlr",       "startTime":"1414828175",       "endTime":"1414828548"}',
        --'{"imei":"676596739220424",   "tokenCode":"RGIfookm46",       "startTime":"1414828180",       "endTime":"1414829347"}',
        --'{"imei":"395592764138151",   "tokenCode":"dzkrDJviS5",       "startTime":"1414828441",       "endTime":"1414870841"}',
        '{"imei":"795374504990001","tokenCode":"lflCw6kKl6","startTime":"1414828560","endTime":"1414937971"}',
        --'{"tokenCode":"nkSki7Egql","imei":"539281561163912","startTime":"1416106282","endTime":"1416107032"}'
        --'{"tokenCode":"IqplXiQv0Q","imei":"199563123936856","startTime":"1415706990","endTime":"1415708124"}'
}

for idx,val in pairs(body) do
	--print(idx,val)
	local data = get_data(val)
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end
