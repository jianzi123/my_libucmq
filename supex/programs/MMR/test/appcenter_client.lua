package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')
--for i=1,100000 do

	local tcp = socket.tcp()
	tcp:setoption('keepalive', true)
	tcp:settimeout(1, 'b')  -- five second timeout

	local ret = tcp:connect("127.0.0.1", 9999)
	if ret == nil then
		return false
	end

        --body = '{"userid":"eB6bYE8pkl","yes":1, "readBizid":"a35a5b9abfc6994165c1134182642f97d5"}'
        --body = '{"userid":"BErsfUt4Dr","yes":true,"bizid":"a38df1ac80becd11e3b1a3002219522239"}'
        body = '{"powerOn":true,"accountID":"zdfeqE74Vi",	"tokenCode":"1adada912939",	"model":"SG900"}'

	--local data = "POST /appcenterApply.json?app_name=reply_parking_show_1&app_mode=alone HTTP/1.0\r\n" ..
	local data = "POST /appcenterApply.json?app_name=a_app_report_user_online&app_mode=alone HTTP/1.0\r\n" ..
                     "User-Agent: curl/7.33.0\r\n" ..
                     "Host: 127.0.0.1:9999\r\n" ..
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
--end
--os.execute("sleep 10")

